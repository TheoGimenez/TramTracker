/*
 * File: /src/DisplayController.h
 * Project: ESP32 TramTracker
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 20th 2026, 1:06:38 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_DISPLAY_CONTROLLER_H_
#define SRC_DISPLAY_CONTROLLER_H_

#include <WString.h> //!STRING
#include <cstdint>
// #include <Arduino.h> //!
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
//
#include "debugLog.h"

// LED Matrix Conf
namespace {
namespace hardware {
constexpr auto    k_hardware_type = MD_MAX72XX::GENERIC_HW;
// constexpr auto    k_hardware_type = MD_MAX72XX::FC16_HW;
constexpr uint8_t k_max_device    =  4; // 8x8 matrix devices to use
// constexpr uint8_t k_data_pin      = 23; // ESP data pin (connect to DIN of matrix)
// constexpr uint8_t k_clk_pin       = 18; // ESP clock pin (connect to CLK oqf matrix)
// constexpr uint8_t k_cs_pin        =  5; // ESP chip select pin (connect to CS/LOAD of matrix)
//                                         // ESP input/output
constexpr uint8_t k_data_pin      = 13; // ESP data pin (connect to DIN of matrix)
constexpr uint8_t k_clk_pin       = 15; // ESP clock pin (connect to CLK oqf matrix)
constexpr uint8_t k_cs_pin        = 14; // ESP chip select pin (connect to CS/LOAD of matrix)
// constexpr uint8_t k_cs_pin        =  4; //! Change cs pin convention to 4 from 5 because of defective pin
} // namespace hardware
} // namespace

class DisplayController
{
public:
    enum RenderState
    {
        Default          = 0,
        TemporaryMessage = 1,
        StaticMessage    = 2,
        render_state_count //* ncaca ot a usable buffer, only used to get number of buffer defined. Should always be the lastest item.
    };

private:
    enum BufferIndex
    {
        MAIN     = 0,
        TEMPORARY= 1,
        WRITABLE = 2,
        buffer_index_count //* not a usable buffer, only used to get number of buffer defined. Should always be the lastest item.
    };
    using RenderHandler = void (DisplayController::*)(void);

    MD_Parola     m_renderer;
    RenderHandler m_renderFunctions[RenderState::render_state_count];

    //* multiple buffer array to avoid segment fault at read when a text is updated.
    //* the buffer at index BufferIndex::WRITABLE (2) will be always the writable buffer, same as BufferIndex::MAIN (0) and BufferIndex::TEMPORARY (1).
    String        m_textBuffers[3];
    String*       m_bufferPtrs[3]; // pointers to the buffers that are currently being used for rendering and writing. The other buffer is free to be written to.
    //
    u_int8_t      m_state;
    u_int16_t     m_displayOccurrenceCountdown;

public:
    DisplayController(void);
    ~DisplayController() = default;
public:
    void begin               (void);
    void updateDisplay       (void);

    void setMainText         (const String& text);
    void setDisplayToMainText(bool forceClear = false);

    void setTemporaryText    (const String& text);
    void setDisplayToTmpText (bool forceClear = false, const u_int16_t occurrenceCount = 1);
   
    void setDisplayToStatic  (bool forceClear = false);

private:
    void _renderDefault      (void);
    void _renderTmp          (void);
    void _renderStatic       (void);
    
    void _swapWritableBuffer (const BufferIndex bufferToSwap);
};

#endif //* SRC_DISPLAY_CONTROLLER_H_
