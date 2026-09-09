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
    m_state{RenderState::Default},
    m_textBuffers{defaultText, defaultText, defaultText},
    m_bufferPtrs {
        &m_textBuffers[WRITABLE_BUFFER],
        &m_textBuffers[MAIN_BUFFER],
        &m_textBuffers[TEMPORARY_BUFFER],
    },
    m_displayOccurrenceCountdown{0}
{
    // MD_Parola m_renderer need to be initialized in setup() function before display anything.
}

void
DisplayController::begin(void)
{
    m_renderer.begin(); // initialization needed to do in setup();
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
    *m_bufferPtrs[WRITABLE_BUFFER] = text;
    _swapWritableBuffer(MAIN_BUFFER);    
}

void
DisplayController::setDisplayToMainText(void)
{
    m_renderer.displayClear();
    m_renderer.displayScroll(m_bufferPtrs[MAIN_BUFFER]->c_str(), PA_CENTER, PA_SCROLL_LEFT, 100);
    m_state = RenderState::Default;
}

void
DisplayController::setTemporaryText(const String& text)
{ 
    *m_bufferPtrs[WRITABLE_BUFFER] = text;
    _swapWritableBuffer(TEMPORARY_BUFFER);
}

void
DisplayController::setDisplayToTmpText(const u_int16_t occurrenceCount)
{
    m_displayOccurrenceCountdown = occurrenceCount;

    m_renderer.displayClear();
    m_renderer.displayScroll(m_bufferPtrs[TEMPORARY_BUFFER]->c_str(), PA_CENTER, PA_SCROLL_LEFT, 100);
    m_state = RenderState::TemporaryMessage;
}

void
DisplayController::setDisplayToStatic(void)
{
    m_renderer.displayClear();
    m_renderer.displayText(m_bufferPtrs[TEMPORARY_BUFFER]->c_str(), PA_LEFT, 70, 1000, PA_NO_EFFECT);
    m_state = RenderState::StaticMessage;
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
        m_renderer.displayReset();
    }
}

void
DisplayController::_renderStatic(void)
{
    m_renderer.displayAnimate();
}

void
DisplayController::_swapWritableBuffer(const int bufferToSwap)
{
    String* tmp = m_bufferPtrs[bufferToSwap];

    m_bufferPtrs[bufferToSwap] = m_bufferPtrs[WRITABLE_BUFFER];
    m_bufferPtrs[WRITABLE_BUFFER] = tmp;
}