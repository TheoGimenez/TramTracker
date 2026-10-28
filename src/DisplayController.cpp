/*
 * File: /src/DisplayController.cpp
 * Project: ESP32 TramTracker
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 28th 2026, 11:23:41 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "DisplayController.h"

DisplayContr::DisplayContr() : m_renderer{
                                   hardware::k_hardware_type,
                                   hardware::k_data_pin,
                                   hardware::k_clk_pin,
                                   hardware::k_cs_pin,
                                   hardware::k_max_device},
                               m_renderFunctions{
                                   &DisplayContr::_renderMain,
                                   &DisplayContr::_renderTmp,
                                   &DisplayContr::_noRender},
                               m_renderBuffers{{0}, {0}},
                               m_strQueueingBuffer{{0}, {0}},
                               m_queuingBufferLen{0},
                               m_renderIterationCountdown{0},
                               m_queueingBufferMutex{xSemaphoreCreateMutex()}
{
    xSemaphoreGive(m_queueingBufferMutex);
    std::strcpy(m_renderBuffers[MAIN], "No data");
}

void
DisplayContr::begin(void)
{
    m_renderer.begin();
    m_renderer.setIntensity(0);
    m_renderer.displayClear();

    renderMainText();
}

void
DisplayContr::updateRender(void)
{
    (this->*m_renderFunctions[m_renderState])();
}

void
DisplayContr::resetRender(void)
{
    m_renderer.displayScroll(
        (m_renderState == RenderState::Main)
            ? m_renderBuffers[MAIN]
            : m_renderBuffers[TEMPORARY],
        PA_CENTER, PA_SCROLL_LEFT, k_main_speed);
}

void
DisplayContr::renderMainText(void)
{
    m_renderer.displayScroll(m_renderBuffers[MAIN], PA_CENTER, PA_SCROLL_LEFT, k_main_speed);
    m_renderState = RenderState::Main;
    updateRender();
}

void
DisplayContr::renderTmpText(bool forceRender, const uint8_t iteration)
{
    _copyQueueingToBuffer(BufferIndex::TEMPORARY);

    if (forceRender)
    {
        m_renderer.displayClear();
    }
    m_renderIterationCountdown = iteration;
    m_renderer.displayScroll(m_renderBuffers[TEMPORARY], PA_CENTER, PA_SCROLL_LEFT, k_tmp_speed);
    m_renderState = RenderState::Temporary;
    updateRender();
}

void
DisplayContr::setMainText(const std::string &text)
{
    const auto lock = LockGuard{m_queueingBufferMutex, MAX_MUTEX_TIMEOUT};
    if (!lock.isLock()) return;

    const auto copied_size = text.copy(m_strQueueingBuffer[MAIN], text.length()); // use text.size() because we are sure bjuffer had enough length

    m_strQueueingBuffer[MAIN][copied_size] = '\0';
    m_queuingBufferLen[MAIN] = text.length();
}


void
DisplayContr::setTmpText(const std::string &text)
{
    const auto lock = LockGuard{m_queueingBufferMutex, MAX_MUTEX_TIMEOUT};
    if (!lock.isLock()) return;

    const auto copied_size = text.copy(m_strQueueingBuffer[TEMPORARY], text.length()); // use text.size() because we are sure bjuffer had enough length

    m_strQueueingBuffer[TEMPORARY][copied_size] = '\0';
    m_queuingBufferLen[TEMPORARY] = text.length();
}

// Private:
bool
DisplayContr::_bufferIsEmpty(const char* buffer)
{
    const auto lock = LockGuard{m_queueingBufferMutex, MAX_MUTEX_TIMEOUT};

    return (buffer[0] == '\0');
}

void
DisplayContr::_renderMain(void)
{
    if (m_renderer.displayAnimate())
    {
        m_renderer.displayReset();

        if (!_bufferIsEmpty(m_strQueueingBuffer[BufferIndex::MAIN]))
        { 
            _copyQueueingToBuffer(BufferIndex::MAIN);
            renderMainText();
        }
    }
}

void
DisplayContr::_renderTmp(void)
{
    if (m_renderer.displayAnimate())
    {
        if (m_renderIterationCountdown--)
        {
            renderMainText();
        }
    }
}

void
DisplayContr::_noRender(void) {} // m_renderer.displayClear();


void
DisplayContr::_copyQueueingToBuffer(const BufferIndex idx)
{
    const auto lock = LockGuard{m_queueingBufferMutex, MAX_MUTEX_TIMEOUT};
    if (!lock.isLock()) return;

    std::strcpy(m_renderBuffers[idx], m_strQueueingBuffer[idx]);
    const auto str_len = m_queuingBufferLen[idx];

    m_renderBuffers[idx][str_len - 1] = '\0';
    m_strQueueingBuffer[idx][0] = '\0';
}
