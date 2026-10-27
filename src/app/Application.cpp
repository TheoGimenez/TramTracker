/*
 * File: /src/app/Application.cpp
 * Project: ESP32 TramTracker
 * Created Date: 18-10-2026, 10:04:39 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 27th 2026, 10:42:47 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "Application.h"

Application::Application() : m_display{},
                             m_webServer{},
                             m_selected_stop{0},
                             m_stop_mutex{xSemaphoreCreateMutex()}
{
    xSemaphoreGive(m_stop_mutex);
}

void
Application::begin()
{
    // const auto selected = transit::resolve_selected_stop("3", "Place Carnot").value();
    // xQueueSend(
        // m_selected_stop_queue,
        // &selected,
        // portTICK_PERIOD_MS * 500);

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

    if (digitalRead(hardware::k_boot_button) == LOW)
    {
        m_display.setTmpText(WiFi.localIP().toString().c_str());
        m_display.renderTmpText(true, 2);
        utils::task_delay(100);
    }
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
    pinMode(hardware::k_boot_button, INPUT_PULLDOWN); // button used to show IP addres on display.

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
    auto resolved_stop = transit::resolve_selected_stop(selectedRouteName, selectedStopName);

    if (resolved_stop.has_value() == true)
    {
        if (xSemaphoreTake(m_stop_mutex, MAX_MUTEX_TIMEOUT) == pdTRUE)
        {
            m_selected_stop = *resolved_stop;
            xSemaphoreGive(m_stop_mutex);
        }

        m_display.setTmpText(std::string("Arret ") + selectedStopName + " selectionne...");
        m_display.renderTmpText(true, 1);
//! FOR DEBUG PURPOSE ONLY
// _stopNextArrivalsService();
//         _stopNextArrivalsService();
//!
        return true;
    }

    return false;
}

std::string
Application::_formatNextArrivalToString(const time_t remainingSecs[2])
{
    std::string display_buffer;

    display_buffer.reserve(512);

    if (xSemaphoreTake(m_stop_mutex, MAX_MUTEX_TIMEOUT) == pdTRUE)
    {
        display_buffer.append(m_selected_stop.route);
        display_buffer.append(" ");
        display_buffer.append(m_selected_stop.name);
        display_buffer.append(":");
        xSemaphoreGive(m_stop_mutex);
    }
    
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
    if (xSemaphoreTake(m_stop_mutex, MAX_MUTEX_TIMEOUT) == pdTRUE)
    {

        const auto next_arrivals = rt::fetch_next_arrival(m_selected_stop.route, m_selected_stop.ids, utils::get_time());
        if (next_arrivals.has_value())
        {
            const auto text_to_display = _formatNextArrivalToString(next_arrivals->seconds_until_arrivals);
            m_display.setMainText(text_to_display);
        }
        else
        {
            m_display.setMainText(std::string(m_selected_stop.route) + " " + m_selected_stop.name + ": " + k_text_no_service);
        }
// m_display.renderMainText();
        xSemaphoreGive(m_stop_mutex);
    }
}
