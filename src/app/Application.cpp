/*
 * File: /src/app/Application.cpp
 * Project: ESP32 TramTracker
 * Created Date: 18-10-2026, 10:04:39 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 20th 2026, 1:29:52 am
 * Modified By: Theo Gimenez
 * -----
 */

#include "Application.h"

void
Application::begin()
{

    _initializeHarware();
    _initializeNetwork();
    _initializeWebServer();


    // Sync time. Needed to use and process gtfs-rt timestamp
    configTzTime(k_ntp_timezone, k_ntp_server); //! need internet
    // if (get_time == 0)
    // {
    //     //? find a way to ensure sync
    // }
}

void
Application::update()
{
    m_display.updateDisplay();

    if (digitalRead(hardware::k_boot_button) == LOW)
    {
        m_display.setTemporaryText(WiFi.localIP().toString());
        m_display.setDisplayToTmpText(true, 2);
        delay(100);
    }
}

// private:
void
Application::_initializeHarware(void)
{
#   ifdef TRAM_DEBUG
    Serial.begin(115200);
#   endif

    pinMode(hardware::k_esp_led, OUTPUT);             // esp32 board LED that blink to advertise specific situation.
    pinMode(hardware::k_boot_button, INPUT_PULLDOWN); // button used to show IP addres on display.

    m_display.begin();
}

void
Application::_initializeNetwork  (void)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(env::k_wifi_ssid, env::k_wifi_password);
    WiFi.setAutoReconnect(true);
    // Blink when attent to connect to wifi
    while (WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(2, HIGH);
        delay(100);
        digitalWrite(2, LOW);
        delay(100);
    }

    LOG_PRINTF("IP=[%s]\n",WiFi.localIP().toString())
    //! app::display.setTemporaryText(WiFi.localIP().toString());

    // network client
    rt::initialize_network();
}

void
Application::_initializeWebServer(void)
{
    m_webServer.begin(WRAP_METHOD(_onStopSelection));
        // [this](const String& s1, const String& s2){return _onStopSelection(s1, s2);});

    // m_webServer.setHomePage(SELECT_STOP_PAGE, [this](String& s1, const String& s2){return _onStopSelection(s1, s2)});
    // m_webServer.setHomePage(WIFI_CONFIG_PAGE, [this](String& s1, const String& s2){return _onWiFiSet(s1, s2)});
}

bool
Application::_onStopSelection(const String &selectedRouteName, const String &selectedStopName)
{
    auto resolved_stop = transit::resolve_selected_stop(selectedRouteName.c_str(), selectedStopName.c_str());

    if (resolved_stop.has_value() == true)
    {
        m_selectedStop = *resolved_stop;

        m_display.setTemporaryText(String("Arret ") + selectedStopName + " selectionne...");
        m_display.setDisplayToTmpText(true, 1);
//! FOR DEBUG PURPOSE ONLY
_getStopNextArrivals();
//!
        return true;
    }

    return false;
}

void
Application::_getStopNextArrivals(void)
{
    const auto next_arrivals = rt::fetch_next_arrival(m_selectedStop.route, m_selectedStop.ids, get_time());
    String     display_buffer;

    display_buffer.reserve(512);

    display_buffer.concat(m_selectedStop.route);
    display_buffer.concat(" ");
    display_buffer.concat(m_selectedStop.name);
    display_buffer.concat(": ");

    if (next_arrivals.has_value())
    {
        const auto &remaining_secs = next_arrivals.value().seconds_until_arrivals;

        if (remaining_secs[k_dir_A])
        {
            display_buffer.concat("SENS_A"); //TODO find terminus name.
            display_buffer.concat("-> ");
            append_num(display_buffer, remaining_secs[k_dir_A] / 60);
            display_buffer.concat(":");
            append_num(display_buffer, remaining_secs[k_dir_A] % 60);
            display_buffer.concat("min");
            // display_buffer.concat(String(remaining_secs[k_dir_A] / 60) + ":" + String(remaining_secs[k_dir_A] % 60) + ". / ");
        }
        else
        {
            display_buffer.concat("SENS_A"); //TODO find terminus name.
            display_buffer.concat("-> ");
            display_buffer.concat(k_text_no_service);
        }

        if (remaining_secs[k_dir_B])
        {
            display_buffer.concat("SENS_B"); //TODO find terminus name.
            display_buffer.concat("-> ");
            append_num(display_buffer, remaining_secs[k_dir_B] / 60);
            display_buffer.concat(":");
            append_num(display_buffer, remaining_secs[k_dir_B] % 60);
            display_buffer.concat("min");
            // display_buffer.concat(String(remaining_secs[k_dir_B] / 60) + ":" + String(remaining_secs[k_dir_B] % 60) + ".");
        }
        else
        {
            display_buffer.concat("SENS_B"); //TODO find terminus name.
            display_buffer.concat("-> ");
            display_buffer.concat(k_text_no_service);
            // display_buffer.concat(k_text_no_service);
        }
    }
    else
    {
        // display_buffer.concat(k_text_no_service ": SENS_B" ": " k_text_no_service);
        display_buffer.concat(k_text_no_service);
    }

    m_display.setMainText(display_buffer);
    m_display.setDisplayToMainText();
}
