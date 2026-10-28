/*
 * File: /src/app/Application.cpp
 * Project: ESP32 TramTracker
 * Created Date: 18-10-2026, 10:04:39 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 28th 2026, 10:51:55 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "Application.h"

Application::Application() : m_display{},
                             m_webServer{},
                             m_selected_stop{0},
                             m_stop_mutex{xSemaphoreCreateMutex()},
                             m_boot_button_pressed{false}
{
}

void
Application::begin()
{
    _initializeHarware();
    _initializeNetwork();
    _initializeWebServer();


    // Sync time. Needed to use and process gtfs-rt timestamp
    utils::configure_tz_time(k_ntp_timezone, k_ntp_server); //! need internet
    // if (get_time == 0)
    // {
    //     //? find a way to ensure sync
    // }
}

void
Application::updateDisplay(void)
{
    m_display.updateRender();

    const bool boot_button_pressed = (digitalRead(hardware::k_boot_button) == LOW);
    if (boot_button_pressed && !m_boot_button_pressed)
    {
        m_display.setTmpText(WiFi.localIP().toString().c_str());
        m_display.renderTmpText(true, 2);
    }
    m_boot_button_pressed = boot_button_pressed;
}

void
Application::updateArrivals(void)
{
    _stopNextArrivalsService();
}

// private:
void
Application::_initializeHarware(void)
{
    pinMode(hardware::k_esp_led, OUTPUT);             // esp32 board LED that blink to advertise specific situation.
    pinMode(hardware::k_boot_button, INPUT_PULLUP); // button used to show IP address on display.

    m_display.begin();
}

void
Application::_initializeNetwork(void)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(env::k_wifi_ssid, env::k_wifi_password);
    WiFi.setAutoReconnect(true);
    // Blink when attent to connect to wifi
    while (WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(2, HIGH);
        utils::task_delay(100);
        digitalWrite(2, LOW);
        utils::task_delay(100);
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
Application::_onStopSelection(const char* selectedRouteName, const char* selectedStopName)
{
    LOG_PRINTF("Stop selection request: route='%s', stop='%s'\n",
               selectedRouteName != nullptr ? selectedRouteName : "<null>",
               selectedStopName != nullptr ? selectedStopName : "<null>")

    if (selectedRouteName == nullptr || selectedStopName == nullptr)
    {
        LOG_PRINTLN("Stop selection rejected: route or stop parameter is null")
        return false;
    }

    auto resolved_stop = transit::resolve_selected_stop(selectedRouteName, selectedStopName);

    if (resolved_stop.has_value() == true)
    {
        {
            const auto lock = LockGuard{m_stop_mutex, MAX_MUTEX_TIMEOUT};
            if (!lock.isLock())
            {
                LOG_PRINTLN("Stop selection rejected: failed to acquire stop mutex")
                return false;
            }

            m_selected_stop = *resolved_stop;
        }

        m_display.setTmpText(std::string("Arret ") + selectedStopName + " selectionne...");
        m_display.renderTmpText(true, 1);

        return true;
    }

    LOG_PRINTF("Stop selection rejected: route '%s' or stop '%s' is unknown or not served\n",
               selectedRouteName,
               selectedStopName)
    return false;
}

std::string
Application::_formatNextArrivalToString(const stop_selection_s& selectedStop, const time_t remainingSecs[2])
{
    std::string display_buffer;

    display_buffer.reserve(512);

    display_buffer.append(selectedStop.route);
    display_buffer.append(" ");
    display_buffer.append(selectedStop.name);
    display_buffer.append(":");
    
    if (remainingSecs[k_dir_A])
    {
        display_buffer.append(" SENS_A"); // TODO find terminus name.
        display_buffer.append("-> ");
        utils::append_num(display_buffer, remainingSecs[k_dir_A] / 60);
        display_buffer.append(":");
        utils::append_num(display_buffer, remainingSecs[k_dir_A] % 60);
        display_buffer.append("min ");
        // display_buffer.append(String(remainingSecs[k_dir_A] / 60) + ":" + String(remainingSecs[k_dir_A] % 60) + ". / ");
    }
    else
    {
        display_buffer.append(" SENS_A"); // TODO find terminus name.
        display_buffer.append("-> ");
        display_buffer.append(k_text_no_service);
    }

    if (remainingSecs[k_dir_B])
    {
        display_buffer.append(" SENS_B"); // TODO find terminus name.
        display_buffer.append("-> ");
        utils::append_num(display_buffer, remainingSecs[k_dir_B] / 60);
        display_buffer.append(":");
        utils::append_num(display_buffer, remainingSecs[k_dir_B] % 60);
        display_buffer.append("min");
        // display_buffer.append(String(remainingSecs[k_dir_B] / 60) + ":" + String(remainingSecs[k_dir_B] % 60) + ".");
    }
    else
    {
        display_buffer.append(" SENS_B"); // TODO find terminus name.
        display_buffer.append("-> ");
        display_buffer.append(k_text_no_service);
        // display_buffer.append(k_text_no_service);
    }

    return display_buffer;
}

void Application::_stopNextArrivalsService(void)
{
    stop_selection_s selected_stop{};
    {
        const auto lock = LockGuard{m_stop_mutex, MAX_MUTEX_TIMEOUT};
        if (!lock.isLock()) return;

        selected_stop = m_selected_stop;
    }

    if (selected_stop.route == nullptr || selected_stop.name == nullptr || (selected_stop.ids[k_dir_A] == 0 && selected_stop.ids[k_dir_B] == 0))
    {
        return;
    }

    const auto next_arrivals = rt::fetch_next_arrival(selected_stop.route, selected_stop.ids, utils::get_time());
    std::string text_to_display;

    if (next_arrivals.has_value())
    {
        text_to_display = _formatNextArrivalToString(selected_stop, next_arrivals->seconds_until_arrivals);
    }
    else
    {
        text_to_display = std::string(selected_stop.route) + " " + selected_stop.name + ": " + k_text_no_service;
    }

    {
        const auto lock = LockGuard{m_stop_mutex, MAX_MUTEX_TIMEOUT};
        if (!lock.isLock()) return;

        // case selection was changed during processing:
        if (m_selected_stop.route != selected_stop.route || m_selected_stop.name != selected_stop.name) // no need to check ids
        {
            return;
        }

        m_display.setMainText(text_to_display);
    }
}
