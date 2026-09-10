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

struct md_parola_initializer_list
{
    MD_MAX72XX::moduleType_t mod;
    uint8_t dataPin;
    uint8_t clkPin;
    uint8_t csPin;
    uint8_t numDevices = 1;
};

class DisplayController
{
    enum BufferIndex
    {
        WRITABLE = 0,
        MAIN     = 1,
        TEMPORARY= 2,
        index_count //* not a usable buffer, only used to get number of buffer defined. Should always be the lastest item.
    };
    enum RenderState
    {
        Default = 0,
        TemporaryMessage = 1,
        StaticMessage = 2
    };
    using RenderHandler = void (DisplayController::*)(void);

private:
    MD_Parola     m_renderer;
    RenderHandler m_renderFunctions[BufferIndex::index_count];

    //* set texts with a multiple buffering to avoid segment fault at read when a text is updated.
    String        m_textBuffers[3];
    String*       m_bufferPtrs[3];
    //
    u_int8_t      m_state;
    u_int16_t     m_displayOccurrenceCountdown;
public:
    DisplayController(md_parola_initializer_list initializer_list, String defaultText);
    ~DisplayController() = default;
public:
    void begin               (void);
    void updateDisplay       (void);

    void setMainText         (const String& text);
    void setDisplayToMainText(void);

    void setTemporaryText    (const String& text);
    void setDisplayToTmpText (const u_int16_t occurrenceCount = 1);
   
    void setDisplayToStatic  (void);

private:
    void _renderDefault      (void);
    void _renderTmp          (void);
    void _renderStatic       (void);
    
    void _swapWritableBuffer (const int bufferToSwap);
};

#endif //* DISPLAY_CONTROLLER_HPP_
