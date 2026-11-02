/*
 * File: /src/app/Application.cpp
 * Project: ESP32 TramTracker
 * Created Date: 18-10-2026, 10:04:39 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Monday, November 2nd 2026, 2:49:43 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "Application.h"


#define TZ_Europe_Paris PSTR("CET-1CEST,M3.5.0,M10.5.0/3")

namespace {

#ifdef TRAM_DEBUG
constexpr uint16_t    k_page_size     = 256 * 4;                   // For case Of Print
#else
constexpr uint16_t    k_page_size     = 256;
#endif
constexpr uint32_t    k_arrival_service_period = 45000;            // 45 seconds.

constexpr const char* k_ntp_timezone    = TZ_Europe_Paris;
constexpr const char* k_ntp_server      = "pool.ntp.org";
constexpr const char* k_text_no_service = "Pas de service.";

DisplayController     display           = {};                      // LED matrix controller
web::AsyncServer      webServer         = {};
stop_selection_s      selected_stop     = {0};
SemaphoreHandle_t     stop_mutex        = xSemaphoreCreateMutex();

namespace hardware {
    constexpr uint8_t k_esp_led = 2;                               // esp integrated blue led
    constexpr uint8_t k_boot_button = 0;                           // esp boot button
} // namespace hardware

void initialize_harware(void)
{
    pinMode(hardware::k_esp_led, OUTPUT);                          // esp32 board LED that blink to advertise specific situation.
    pinMode(hardware::k_boot_button, INPUT_PULLUP);                // button used to show IP address on display.

    display.begin();
}

void initialize_network(void)
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

bool on_stop_selection(const char* selectedRouteName, const char* selectedStopName)
{
    if (selectedRouteName == nullptr || selectedStopName == nullptr)
    {
        LOG_PRINTLN("Stop selection rejected: route or stop parameter is null")
        return false;
    }

    const auto resolved_stop = transit::resolve_selected_stop(selectedRouteName, selectedStopName);

    if (resolved_stop.has_value() == true)
    {
        {
            const auto lock = LockGuard{stop_mutex, MAX_MUTEX_TIMEOUT};
            if (!lock.isLock())
            {
                LOG_PRINTLN("Stop selection rejected: failed to acquire stop mutex")
                return false;
            }

            selected_stop = *resolved_stop;
        }

        display.setTmpText(std::string("Arret ") + selectedStopName + " selectionne...");
        display.renderTmpText(true, 1);

        return true;
    }

    return false;
}

void initialize_web_server(void)
{
    webServer.begin(on_stop_selection);
        // [this](const String& s1, const String& s2){return _onStopSelection(s1, s2);});

    // m_webServer.setHomePage(SELECT_STOP_PAGE, [this](String& s1, const String& s2){return _onStopSelection(s1, s2)});
    // m_webServer.setHomePage(WIFI_CONFIG_PAGE, [this](String& s1, const String& s2){return _onWiFiSet(s1, s2)});
}

std::string format_next_arrival(const stop_selection_s& selectedStop, const arrival_update_s& nextArrivals)
{
    std::string display_buffer;

    display_buffer.reserve(512);

    display_buffer.append(selectedStop.route);
    display_buffer.append(" ");
    display_buffer.append(selectedStop.name);
    display_buffer.append("  ");

    if (nextArrivals.seconds_until_arrivals[k_dir_A])
    {
        display_buffer.append(transit::get_stop_name(nextArrivals.terminus_ids[k_dir_A]));
        display_buffer.append(": ");
        utils::append_num(display_buffer, nextArrivals.seconds_until_arrivals[k_dir_A] / 60);
        display_buffer.append("min ");
    }
    else
    {
        // display_buffer.append(" ");
        display_buffer.append(transit::get_stop_name(nextArrivals.terminus_ids[k_dir_A]));
        display_buffer.append(": ");
        display_buffer.append(k_text_no_service);
    }

    if (nextArrivals.seconds_until_arrivals[k_dir_B])
    {
        display_buffer.append(" - ");
        display_buffer.append(transit::get_stop_name(nextArrivals.terminus_ids[k_dir_B]));
        display_buffer.append(": ");
        utils::append_num(display_buffer, nextArrivals.seconds_until_arrivals[k_dir_B] / 60);
        display_buffer.append("min");
    }
    else
    {
        display_buffer.append(" - ");
        display_buffer.append(transit::get_stop_name(nextArrivals.terminus_ids[k_dir_B]));
        display_buffer.append(": ");
        display_buffer.append(k_text_no_service);
    }

    return display_buffer;
}
} // namespace

void update_next_arrivals(void*)
{
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(k_arrival_service_period)))
    {
        stop_selection_s selection{};
        {
            const auto lock = LockGuard{stop_mutex, MAX_MUTEX_TIMEOUT};
            if (!lock.isLock()) continue;

            selection = selected_stop;
        }

        if (selection.route == nullptr || selection.name == nullptr || (selection.ids[k_dir_A] == 0 && selection.ids[k_dir_B] == 0))
        {
            continue;
        }

        const auto next_arrivals = rt::fetch_next_arrival(selection.route, selection.ids, utils::get_time());
        std::string text_to_display;

        if (next_arrivals.has_value())
        {
            text_to_display = format_next_arrival(selection, *next_arrivals);
        }
        else
        {
            text_to_display = std::string(selection.route) + " " + selection.name + ": " + k_text_no_service;
        }

        {
            const auto lock = LockGuard{stop_mutex, MAX_MUTEX_TIMEOUT};
            
            // Skip a result if the selection changed while the feed was being fetched.
            if (lock.isLock()
                && selected_stop.route == selection.route
                && selected_stop.name == selection.name)
            {
                display.setMainText(text_to_display);
            }
        }

        ;
    }
}

void
update_display(void*)
{
    bool boot_button_pressed = false;

    for (;;)
    {
        display.updateRender();

        const bool boot_button_is_pressed = (digitalRead(hardware::k_boot_button) == LOW);
        if (boot_button_is_pressed && !boot_button_pressed)
        {
            display.setTmpText(WiFi.localIP().toString().c_str());
            display.renderTmpText(true, 2);
        }
        boot_button_pressed = boot_button_is_pressed;

        utils::task_delay(1);
    }
}

void
Application::begin(void)
{
    initialize_harware();
    initialize_network();
    initialize_web_server();

    // Sync time. Needed to use and process gtfs-rt timestamp
    utils::configure_tz_time(k_ntp_timezone, k_ntp_server); //! need internet
    // if (get_time == 0)
    // {
    //     //? find a way to ensure sync
    // }


    xTaskCreatePinnedToCore(
        &update_next_arrivals,
        "nextArrivalService",
        k_page_size * 32, // 8Ko stackDepth if use arduino / espressif value in Byte if use pure freeRtos value in Word (4 bytes)
        nullptr,
        1, // task_priority
        nullptr,
        1 // use core 1 to set More
    );

    xTaskCreatePinnedToCore(
        &update_display,
        "StopDisplayService",
        k_page_size * 8, // 2Ko stackDepth if use arduino / espressif
        nullptr,
        1, // task_priority
        nullptr,
        1 // use core 1 to set More
    );
}
