
#include "networkUtils.h"

#include <WiFiClientSecure.h>
#include "generated_app_config.h"

namespace app
{
    static WiFiClientSecure client;
//     namespace network
//     {
//         enum Status
//         {
//             FAILED = 0,
//             SUCCESS = 1
//         };
//     } // namespace network
}

void
initializeNetwork(void)
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
        Serial.println("Failed to connect");
    }
Serial.println(ok);
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
getResponseHeaderParams(int& content_length)
{
    const uint32_t deadline = millis() + 5000;
    bool first_line = true;

    while (millis() < deadline)
    {
        if (!app::client.available())
        {
            if (!app::client.connected())
            {
                Serial.println("Connection closed before HTTP headers");
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

    Serial.println("Timeout waiting for HTTP headers");
    return false;
}

void
NETWORKUTILS_TEST(void)
{
    int content_length = 0;

    if (!openGTFSConnection())
    {
        Serial.println("Cannot establish connection");
        return;
    }

Serial.println("Connection established");

Serial.print("client available 1: ");
Serial.println(app::client.available());
    sendRequest(app::kGtfsTripUpdateEndpoint);
    // if (!app::client.connected())
    // {
    //     Serial.println("Connection lost");
    //     return closeGTFSConnection();
    // }

Serial.print("client available 3: ");
Serial.println(app::client.available());
    if (!getResponseHeaderParams(content_length))
    {
        Serial.println("No header found");
        return closeGTFSConnection();
    }
    if (content_length == 0)
    {
        Serial.println("No content-length");
        return closeGTFSConnection();
    }
    closeGTFSConnection();
}