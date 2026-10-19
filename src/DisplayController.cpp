/*
 * File: /src/DisplayController.cpp
 * Project: ESP32 TramTracker
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Monday, October 19th 2026, 11:17:25 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "DisplayController.h"

DisplayController::DisplayController(void) : m_renderer{
                                                 hardware::k_hardware_type,
                                                 hardware::k_max_device,
                                                 hardware::k_data_pin,
                                                 hardware::k_clk_pin,
                                                 hardware::k_cs_pin},
                                             m_renderFunctions{
                                                 &DisplayController::_renderDefault,
                                                 &DisplayController::_renderTmp,
                                                 &DisplayController::_renderStatic},
                                             m_textBuffers{
                                                 {"No data"},
                                                 {},
                                                 {}},
                                             m_bufferPtrs{
                                                 &m_textBuffers[BufferIndex::MAIN],
                                                 &m_textBuffers[BufferIndex::TEMPORARY],
                                                 &m_textBuffers[BufferIndex::WRITABLE]},
                                             m_displayOccurrenceCountdown{0}
{
    // MD_Parola m_renderer need to be initialized in setup() function before display anything.
}

void
DisplayController::begin(void)
{
    m_renderer.begin();
    m_renderer.setIntensity(0);
    m_renderer.displayClear();

    setDisplayToMainText();
}

void
DisplayController::updateDisplay(void)
{
    (this->*m_renderFunctions[m_state])();
}

void
DisplayController::setMainText(const String &text)
{
    *m_bufferPtrs[BufferIndex::WRITABLE] = text;
    _swapWritableBuffer(BufferIndex::MAIN);
    m_bufferPtrs[BufferIndex::WRITABLE]->clear();
}

void
DisplayController::setDisplayToMainText(bool forceClear)
{
    m_renderer.displayScroll(m_bufferPtrs[BufferIndex::MAIN]->c_str(), PA_CENTER, PA_SCROLL_LEFT, 30);
    m_state = RenderState::Default;
    
    if (forceClear) {
        m_renderer.displayClear();
    }
    updateDisplay();
}

void
DisplayController::setTemporaryText(const String& text)
{ 
    *m_bufferPtrs[BufferIndex::WRITABLE] = text;
    _swapWritableBuffer(BufferIndex::TEMPORARY);
    m_bufferPtrs[BufferIndex::WRITABLE]->clear();
}

void
DisplayController::setDisplayToTmpText(bool forceClear, const u_int16_t occurrenceCount)
{
    m_displayOccurrenceCountdown = occurrenceCount;
    
    if (forceClear) {
        m_renderer.displayClear();
    }

    m_renderer.displayScroll(m_bufferPtrs[BufferIndex::TEMPORARY]->c_str(), PA_CENTER, PA_SCROLL_LEFT, 70);
    m_state = RenderState::TemporaryMessage;

    updateDisplay();
}

void
DisplayController::setDisplayToStatic(bool forceClear)
{
    if (forceClear) {
        m_renderer.displayClear();
    }

    m_renderer.displayText(m_bufferPtrs[BufferIndex::TEMPORARY]->c_str(), PA_LEFT, 70, 1000, PA_NO_EFFECT);
    m_state = RenderState::StaticMessage;

    updateDisplay();
}

// private:
void
DisplayController::_renderDefault(void)
{
    if (m_renderer.displayAnimate())
    {
LOG_PRINTLN(m_bufferPtrs[DisplayController::BufferIndex::MAIN]->c_str())
        m_renderer.displayReset();
    }
}

void
DisplayController::_renderTmp(void)
{
    if (m_renderer.displayAnimate())
    {
LOG_PRINTLN(m_bufferPtrs[DisplayController::BufferIndex::TEMPORARY]->c_str())
        if (m_displayOccurrenceCountdown == 0)
        {
            return setDisplayToMainText();
        }
        --m_displayOccurrenceCountdown;
    }
}

void
DisplayController::_renderStatic(void)
{
    m_renderer.displayAnimate();
}

void
DisplayController::_swapWritableBuffer(const BufferIndex bufferToSwap)
{
    assert(bufferToSwap < BufferIndex::buffer_index_count);
    String* tmp = m_bufferPtrs[bufferToSwap];

    m_bufferPtrs[bufferToSwap] = m_bufferPtrs[BufferIndex::WRITABLE];
    m_bufferPtrs[BufferIndex::WRITABLE] = tmp;
}
