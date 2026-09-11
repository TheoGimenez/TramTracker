/*
 * Filename: /home/theo/Documents/PlatformIO/Projects/TramTracker/src/main.cpp
 * Path: /home/theo/Documents/PlatformIO/Projects/TramTracker/src
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 *
 * Copyright (c) 2026 Your Company
 */

#include <Arduino.h>

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include <WiFi.h>

#include "DisplayController.h"
#include "webServer.h"

// LED Matrix Conf
constexpr auto k_hardware_type = MD_MAX72XX::FC16_HW;
constexpr uint8_t k_max_device =  4; // 8x8 matrix devices to use
constexpr uint8_t k_data_pin   = 23; // ESP data pin (connect to DIN of matrix)
constexpr uint8_t k_clk_pin    = 18; // ESP clock pin (connect to CLK oqf matrix)
constexpr uint8_t k_cs_pin     =  5; // ESP chip select pinm (connect to CS/LOAD of matrix)
                                     // ESP input/output
constexpr uint8_t k_esp_led    =  2; // esp integrated blue led
constexpr uint8_t k_boot_button=  0; // esp boot button


// WiFi Access Point Settings
struct {
    const char *ssid;
    const char *password;
} const wifiCredentials PROGMEM = {
    WIFI_SSID,
    WIFI_PASSWORD
};
//
struct {
    DisplayController display; // LED Matrix Controller
    WebServer webserver; // Webserver to control the display
} components = {{}, {}};

// void onTramLineSelected(const uint32_t selectedLine)
// {
//     selectedLine = selectedLine;

// }

void setup()
{
    Serial.begin(115200);
    pinMode(k_esp_led, OUTPUT);
    pinMode(k_boot_button, INPUT_PULLDOWN);
    //
    //* Setup display
    components.display.begin();
    //
    //* Setup wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiCredentials.ssid, wifiCredentials.password);
    while (WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(2, HIGH);
        delay(100);
        digitalWrite(2, LOW);
        delay(100);
    }
    components.display.setTemporaryText(WiFi.localIP().toString());
    //
    //* Setup webserver
    components.webserver.begin([](String selectedLine) {
        components.display.setMainText(selectedLine);
        components.display.setDisplayToMainText();
    });
    //
    // setup socket
}

void loop()
{
    components.display.updateDisplay();

    if (digitalRead(k_boot_button) == LOW)
    {
        components.display.setDisplayToTmpText(true, 2);
        delay(100);
    }
}