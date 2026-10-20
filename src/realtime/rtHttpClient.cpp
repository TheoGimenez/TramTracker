/*
 * File: /src/realtime/rtHttpClient.cpp
 * Project: ESP32 TramTracker
 * Created Date: 19-10-2026, 9:27:23 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Monday, October 19th 2026, 11:40:30 pm
 * Modified By: Theo Gimenez
 * -----
 */


#include "rtHttpClient.h"

namespace rt
{
    static WiFiClientSecure client;
}

void
rt::initialize_network(void)
{
    rt::client.setInsecure();
    rt::client.setTimeout(5); // 5s timeout
}

int
rt::open_gtfs_connection(void)
{
    rt::client.stop();

    int ok = rt::client.connect(env::k_gtfs_base_url, env::k_gtfs_port);
    if (!ok)
    {
        LOG_PRINTLN("Failed to connect")
    }
    LOG_PRINTLN(ok)
    return ok;
}

void
rt::close_gtfs_connection(void)
{
    rt::client.stop();
}

void
rt::send_request(const char *endpoint)
{
    rt::client.print(F("GET "));
    rt::client.print(endpoint);
    rt::client.println(F(" HTTP/1.1"));
    rt::client.print(F("Host: "));
    rt::client.println(env::k_gtfs_base_url);
    rt::client.println(F("Accept: application/octet-stream"));
    rt::client.println(F("Connection: close"));
    rt::client.println();
}

uint32_t 
rt::get_content_length(void)
{
    const uint32_t deadline       = millis() + 5000;
    uint32_t       content_length = 0;

    while (millis() < deadline)
    {
        if (!rt::client.available())
        {
            if (!rt::client.connected())
            {
                LOG_PRINTLN("Connection closed before HTTP headers")
                return content_length;
            }
            delay(10);
            continue;
        }

        String line = rt::client.readStringUntil('\n');

        if (line == "\r") // header done
        {
            return content_length;
        }
        
        if (line.startsWith(F("Content-Length:")))
        {
            content_length = line.substring(16).toInt();
        }
    }

    LOG_PRINTLN("Timeout waiting for HTTP headers")
    return content_length;
}

bool
rt::read_response_bytes(uint8_t* buffer, size_t count)
{
    const uint32_t timeout    = millis() + 5000;
    size_t         total_read = 0;

    while (total_read < count)
    {
        if (millis() > timeout)
        {
            return false;
        }

        if (rt::client.available())
        {
            int32_t byte_read = rt::client.read(buffer + total_read, count - total_read);

            if (byte_read < 0)
            {
                return false;
            }
            total_read += size_t(byte_read);
        }
        else if (!rt::client.connected())
        {
            // brutal disconnection.
            return false;
        }
    }
    return true;
}
