
#include "networkUtils.h"

#include "utils.h"
#include <WiFiClientSecure.h>
#include "generated_app_config.h"

namespace app
{
    static WiFiClientSecure client;
}

void initializeNetwork(void)
{
    app::client.setInsecure();
    app::client.setTimeout(8); // 8s timeout
}

static int
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

static void
closeGTFSConnection(void)
{
    app::client.stop();
}

static void
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

static bool
getResponseHeaderParams(int &content_length)
{
    const uint32_t deadline = millis() + 5000;
    bool first_line = true;

    while (millis() < deadline)
    {
        if (!app::client.available())
        {
            if (!app::client.connected())
            {
                LOG_PRINTLN("Connection closed before HTTP headers")
                return false;
            }
            delay(10);
            continue;
        }

        String line = app::client.readStringUntil('\n');

        if (line == "\r") // header done
        {
            return true;
        }

        if (line.startsWith(F("Content-Length:")))
        {
            content_length = line.substring(16).toInt();
        }
    }

    LOG_PRINTLN("Timeout waiting for HTTP headers")
    return false;
}

void NETWORKUTILS_TEST(void)
{
    int content_length = 0;

    if (!openGTFSConnection())
    {
        LOG_PRINTLN("Cannot establish connection")
        return;
    }
    LOG_PRINTLN("Connection established")

    sendRequest(app::kGtfsTripUpdateEndpoint);
    // if (!app::client.connected())
    // {
    //     LOG_PRINTLN("Connection lost")
    //     return closeGTFSConnection();
    // }

    if (!getResponseHeaderParams(content_length))
    {
        LOG_PRINTLN("No header found")
        return closeGTFSConnection();
    }
    if (content_length == 0)
    {
        LOG_PRINTLN("No content-length")
        return closeGTFSConnection();
    }
    closeGTFSConnection();
}