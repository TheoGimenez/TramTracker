/*
 * Filename: /home/theo/Documents/PlatformIO/Projects/TramTracker/src/DisplayController.h
 * Path: /home/theo/Documents/PlatformIO/Projects/TramTracker/src
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * 
 * Copyright (c) 2026 Your Company
 */

#ifndef DISPLAY_CONTROLLER_HPP_
#define DISPLAY_CONTROLLER_HPP_

#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// LED Matrix Conf
namespace hardware {
constexpr auto k_hardware_type = MD_MAX72XX::FC16_HW;
constexpr uint8_t k_max_device =  4; // 8x8 matrix devices to use
constexpr uint8_t k_data_pin   = 23; // ESP data pin (connect to DIN of matrix)
constexpr uint8_t k_clk_pin    = 18; // ESP clock pin (connect to CLK oqf matrix)
constexpr uint8_t k_cs_pin     =  5; // ESP chip select pinm (connect to CS/LOAD of matrix)
                                     // ESP input/output
}
class DisplayController
{
    enum BufferIndex
    {
        MAIN     = 0,
        TEMPORARY= 1,
        WRITABLE = 2,
        index_count //* not a usable buffer, only used to get number of buffer defined. Should always be the lastest item.
    };
    using RenderHandler = void (DisplayController::*)(void);

    MD_Parola     m_renderer;
    RenderHandler m_renderFunctions[BufferIndex::index_count];

    //* multiple buffer array to avoid segment fault at read when a text is updated.
    //* the buffer at index BufferIndex::WRITABLE (2) is always the writable buffer, the buffer at index 1 is always the main text buffer, and the buffer at index 2 is always the temporary text buffer.
    String        m_textBuffers[3];
    String*       m_bufferPtrs[3]; // pointers to the buffers that are currently being used for rendering and writing. The other buffer is free to be written to.
    //
    u_int8_t      m_state;
    u_int16_t     m_displayOccurrenceCountdown;

public:
    enum RenderState
    {
        Default = 0,
        TemporaryMessage = 1,
        StaticMessage = 2
    };
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

#endif //* DISPLAY_CONTROLLER_HPP_
