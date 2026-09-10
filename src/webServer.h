#ifndef WEBPAGE_H_
#define WEBPAGE_H_

#include <functional>

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>TramTraker</title>
    <style>
        body{font-family: Arial, sans-serif; text-align: center;}
        input, select, button {font-size: 16px; margin: 10px; padding: 8px;}
    </style>
</head>
<body>
    <h2>Tram Tracker</h2>
    <label for="tramLine">Selectionner Arret:</label><br>
    <select class="tramLine" onchange="selectLine(this.value)">
        <option value="" disabled>-- Choisir une ligne --</option>
        <option value="1">T1 - Mosson <-> Gare Sud de France</option>
        <option value="2">T2 - St-Jean de Védas Centre <-> Jacou</option>
        <option value="3" selected>T3 - Juvignac <-> Lattes / Pérols</option>
        <option value="4">T4 - Garcia Lorca <-> Garcia Lorca</option>
        <option value="5">T5 - Clapiers <-> Grès de Montpellier</option>
    </select>
    <script>
        function selectLine(line) {
            var xhr = new XMLHttpRequest();
            if (line) {
                xhr.open("GET","/update?tramline="+line,true);
            } else {
                xhr.open("GET","/update?tramline=null",true);
            }
            xhr.send();
        }
    </script>
</body>
</html>
)rawliteral";
const char * const TRAM_SELECTOR_PARAM_INPUT = "tramline";


class WebServer
{
    AsyncWebServer server;
    std::function<void(const String&)> m_onUpdateCallback;

public:
    WebServer(uint16_t port);
    ~WebServer() = default;
public:
    void        begin           (std::function<void(const String&)> onUpdateCallback);

private:
    static void _homePageHandler(AsyncWebServerRequest *request);
    void        _onUpdateHandler(AsyncWebServerRequest *request);

};

#endif /* WEBPAGE_H_ */