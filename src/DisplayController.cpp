/*
 * Filename: /home/theo/Documents/PlatformIO/Projects/TramTracker/src/DisplayController.cpp
 * Path: /home/theo/Documents/PlatformIO/Projects/TramTracker/src
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * 
 * Copyright (c) 2026 Your Company
 */

#include "DisplayController.h"

DisplayController::DisplayController(md_parola_initializer_list initializer_list, String defaultText)
    : m_renderer{
        initializer_list.mod,
        initializer_list.dataPin,
        initializer_list.clkPin,
        initializer_list.csPin,
        initializer_list.numDevices
    },
    m_renderFunctions{&DisplayController::_renderDefault, &DisplayController::_renderTmp, &DisplayController::_renderStatic},
    m_textBuffers{defaultText, defaultText, defaultText},
    m_bufferPtrs {
        &m_textBuffers[BufferIndex::WRITABLE],
        &m_textBuffers[BufferIndex::MAIN],
        &m_textBuffers[BufferIndex::TEMPORARY],
    },
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

    m_renderer.displayScroll(m_bufferPtrs[BufferIndex::TEMPORARY]->c_str(), PA_CENTER, PA_SCROLL_LEFT, 70);
    m_state = RenderState::TemporaryMessage;
    
    if (forceClear) {
        m_renderer.displayClear();
    }
    updateDisplay();
}

void
DisplayController::setDisplayToStatic(bool forceClear)
{
    m_renderer.displayText(m_bufferPtrs[BufferIndex::TEMPORARY]->c_str(), PA_LEFT, 70, 1000, PA_NO_EFFECT);
    m_state = RenderState::StaticMessage;

    if (forceClear) {
        m_renderer.displayClear();
    }
    updateDisplay();
}

// private:
void
DisplayController::_renderDefault(void)
{
    if (m_renderer.displayAnimate())
    {
        m_renderer.displayReset();
    }
}

void
DisplayController::_renderTmp(void)
{
    if (m_renderer.displayAnimate())
    {
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
    assert(bufferToSwap < BufferIndex::index_count);
    String* tmp = m_bufferPtrs[bufferToSwap];

    m_bufferPtrs[bufferToSwap] = m_bufferPtrs[BufferIndex::WRITABLE];
    m_bufferPtrs[BufferIndex::WRITABLE] = tmp;
}
