/*
 * File: /src/networkUtils.cpp
 * Project: ESP32 TramTracker
 * Created Date: 17-09-2026, 3:46:21 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 6th 2026, 12:11:34 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "networkUtils.h"

#include "utils.h"
#include <WiFiClientSecure.h>

#include "generated_app_config.h"

namespace app
{
    static WiFiClientSecure client;
}

void
initializeNetwork(void)
{
    app::client.setInsecure();
    app::client.setTimeout(8); // 8s timeout
}

int
openGTFSConnection(void)
{
    app::client.stop();

    int ok = app::client.connect(app::kGtfsBaseUrl, app::kGtfsPort);
    if (!ok)
    {
        LOG_PRINTLN("Failed to connect")
    }
    LOG_PRINTLN(ok)
    return ok;
}

void
closeGTFSConnection(void)
{
    app::client.stop();
}

void
sendRequest(const char *endpoint)
{
    // app::client.printf(F(
    //     "GET %s HTTP/1.1\r\n"
    //     "Host: "GTFS_BASE_URL"\r\n"
    //     "Accept: application/octet-stream\r\n"
    //     "Connection: close\r\n\r\n"),
    //     endpoint);
    app::client.print(F("GET "));
    app::client.print(endpoint);
    app::client.println(F(" HTTP/1.1"));
    app::client.print(F("Host: "));
    app::client.println(app::kGtfsBaseUrl);
    app::client.println(F("Accept: application/octet-stream"));
    app::client.println(F("Connection: close"));
    app::client.println();
}

uint32_t 
getContentLength(void)
{
    const uint32_t deadline = millis() + 5000;
    uint32_t content_length = 0;

    while (millis() < deadline)
    {
        if (!app::client.available())
        {
            if (!app::client.connected())
            {
                LOG_PRINTLN("Connection closed before HTTP headers")
                return content_length;
            }
            delay(10);
            continue;
        }

        String line = app::client.readStringUntil('\n');

        if (line == "\r") // header done
        {
            return content_length;
        }
LOG_PRINTLN(line)
        if (line.startsWith(F("Content-Length:")))
        {
            content_length = line.substring(16).toInt();
        }
    }

    LOG_PRINTLN("Timeout waiting for HTTP headers")
    return content_length;
}

bool
readResponseBytes(uint8_t* buffer, size_t count)
{
    const uint32_t timeout = millis() + 5000;
    size_t total_read = 0;

    while (total_read < count)
    {
        if (millis() > timeout)
        {
            return false;
        }

        if (app::client.available())
        {
            int byte_read = app::client.read(buffer + total_read, count - total_read);

            if (byte_read < 0)
            {
                return false;
            }
            total_read += size_t(byte_read);
        }
        else if (!app::client.connected())
        {
            // brutal disconnection.
            return false;
        }
    }
    return true;
}
