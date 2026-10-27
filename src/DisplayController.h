/*
 * File: /src/DisplayController.h
 * Project: ESP32 TramTracker
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 27th 2026, 10:31:06 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include <cstdint>
#include <cstring>
#include <string>
//
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
//
#include "debugLog.h"
#include "LockGuard.h"
#include "utils.h"

// LED Matrix Conf
namespace {
namespace hardware {
constexpr auto    k_hardware_type = MD_MAX72XX::FC16_HW;
constexpr uint8_t k_max_device    =  4; // 8x8 matrix devices to use
constexpr uint8_t k_data_pin      = 23; // ESP data pin (connect to DIN of matrix)
constexpr uint8_t k_clk_pin       = 18; // ESP clock pin (connect to CLK oqf matrix)
constexpr uint8_t k_cs_pin        =  5; // ESP chip select pin (connect to CS/LOAD of matrix)
                                        // ESP input/output
} // namespace hardware
constexpr uint16_t k_buffer_size  = 256;
constexpr uint8_t  k_main_speed   = 30;
constexpr uint8_t  k_tmp_speed    = 70;
} // namespace

class DisplayContr
{
public:
    enum RenderState : uint8_t
    {
        Main      = 0,
        Temporary = 1,
        NoRender  = 2,
        render_state_count
    };

private:
    enum BufferIndex : uint8_t
    {
        MAIN  = 0,
        TEMPORARY= 1,
        buffer_index_count //* not a usable buffer, only used to get number of buffer defined. Should always be the lastest item.
    };

    using RenderFct = void (DisplayContr::*)(void);

    MD_Parola         m_renderer;
    RenderFct         m_renderFunctions[render_state_count];

    char              m_renderBuffers[buffer_index_count][k_buffer_size];
    char              m_strQueueingBuffer[buffer_index_count][k_buffer_size];
    // char*          m_bufferPtrs[buffer_index_count];
    RenderState       m_renderState;
    uint16_t          m_renderIterationCountdown;
    //
    SemaphoreHandle_t m_queueingBufferMutex;
public:
    DisplayContr              ();

public:
    void begin                (void);
    void updateRender         (void);
    void resetRender          (void);
    void renderMainText       (void);
    void renderTmpText        (bool forceRender, const uint8_t iteration);
    void setMainText          (const std::string& text);
    void setTmpText           (const std::string& text);

private:
    void _renderMain          (void);
    void _renderTmp           (void);
    void _noRender            (void);// m_renderer.displayClear();

    // private utils:
    bool _bufferIsEmpty       (const char* buffer);
    void _copyQueueingToBuffer(const BufferIndex idx);

    // void            _swapWritableBuffer(const BufferIndex bufferToSwap) {
    //     char* const tmp = m_bufferPtrs[bufferToSwap];

    //     m_bufferPtrs[bufferToSwap] = m_bufferPtrs[WRITABLE];
    //     m_bufferPtrs[WRITABLE]     = tmp;
    // }
};