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

#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "DisplayController.h"

String processor(const String&);
String outputState(int);


// LED Matrix Conf
constexpr auto k_hardware_type = MD_MAX72XX::FC16_HW;
constexpr uint8_t k_max_device = 4;  // 8x8 matrix devices to use
constexpr uint8_t k_data_pin   = 23; // ESP data pin (connect to DIN of matrix)
constexpr uint8_t k_clk_pin    = 18; // ESP clock pin (connect to CLK oqf matrix)
constexpr uint8_t k_cs_pin     = 5;  // ESP chip select pinm (connect to CS/LOAD of matrix)
                                     // ESP input/output
constexpr uint8_t k_esp_led     = 2; // esp integrated blue led
constexpr uint8_t k_boot_button = 0; // esp boot button


// LED Matrix Controller
DisplayController g_displayController{
    {k_hardware_type,
     k_data_pin,
     k_clk_pin,
     k_cs_pin,
     k_max_device},
    String{"No Data"}};

// flag to use from web update to reboot the ESP
bool shouldReboot = false;

// WiFi Access Point Settings
const char *const ssid = WIFI_SSID;
const char *const wifi_password = WIFI_PASSWORD;

// Web Server on port 80
AsyncWebServer server(80);
//! AsyncWebSocket ws("/ws");

// HTML Web Page

// Store HTML contents in PROGMEM (force to store in flash) to save SRAM
const char index_html[] PROGMEM = R"rawliteral(
 <!DOCTYPE html>
 <html>
 <head>
   <title>Tramway Screen Traker</title>
   <style>
     body {font-family: Arial, sans-serif; text-align: center;}
     input, select, button { font-size: 16px; margin: 10px; padding: 8px; }
   </style>
 </head>
 <body>
   <h2>TEST MATRIX Control</h2>
   <form action="/update" method="GET">
     <label>Enter Text:</label>
     <input type="text" name="text" placeholder="Enter message" required>
     <br>
     <label>Scroll Spedd:</label>
     <select name="speed">
       <option value"30">Fast</option>
     </select>
   </form>
   
 <h2>Tram Tracker</h2>
 <p>Enter the tram number and the direction to get the next arrival time.</p>
  %BUTTONPLACEHOLDER%

<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}

</script>
  </body>
 <html>
 )rawliteral";

// Replaces placeholder with button section in your web page

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";
String processor(const String& var) {
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 4</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 33</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"33\" " + outputState(33) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}

String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}


void setup()
{
    Serial.begin(115200);
    pinMode(k_esp_led, OUTPUT);
    pinMode(k_boot_button, INPUT_PULLDOWN);

    //* Setup display
    g_displayController.begin(); // initialization needed to be called sin setup();

    //* Setup wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, wifi_password);
    Serial.print("\nConnecting to WiFi ");
    Serial.println(ssid);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        digitalWrite(2, HIGH);
        delay(100);
        digitalWrite(2, LOW);
        delay(100);
    }
    // Serial.println("\nConnected ! Device IP:");
    // Serial.print("Local ESP32 IP:");
    // Serial.println(WiFi.localIP().toString());
    g_displayController.setTemporaryText(WiFi.localIP().toString());

    //* Setup webserver
    // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        String inputMessage1;
        String inputMessage2;
        
        // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
        if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2))
        {
            inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
            inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
            digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
        }
        else
        {
            inputMessage1 = "No message sent";
        inputMessage2 = "No message sent";
        }

        Serial.print("GPIO: ");
        Serial.print(inputMessage1);
        Serial.print(" - Set to: ");
        Serial.println(inputMessage2);

        request->send(200, "text/plain", "OK");
    });

    // start server
    server.begin();

    // setup socket
}

void loop()
{
    g_displayController.updateDisplay();

    if (digitalRead(k_boot_button) == LOW)
    {
        g_displayController.setDisplayToTmpText();
        delay(100);
    }

    /*
     * Au passage, si tu connais la bibliothèque LVGL, tu retrouveras une philosophie proche : le loop()
     * ne contient quasiment aucune logique, il se contente d'appeler une fonction de mise à jour,
     * et toute la machine à états est encapsulée dans les objets eux-mêmes.
     * C'est une architecture qui fonctionne très bien sur ESP32.
     */
}