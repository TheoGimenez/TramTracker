/*
 * Filename: /src/main.cpp
 * Project: ESP32 TramTracker
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Monday, October 19th 2026, 8:41:12 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "app/Application.h"

namespace {
    Application app;
} // namespace

void setup()
{
    app.begin();
}

void loop()
{
    app.update();
}