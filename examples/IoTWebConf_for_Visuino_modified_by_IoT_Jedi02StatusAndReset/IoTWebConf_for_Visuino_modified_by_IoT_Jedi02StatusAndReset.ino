/**
 * IoTJediWebConfSettings.h -- IoTJediWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/BadASszZ/IoTWebConf_for_Visuino_modified_by_IoT_Jedi
 * --- Based on https://github.com/prampec/IotWebConf ---
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 * 
 * Changed by IoT_Jedi to work together with Visuino
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: Status and Reset
 * Description:
 *   This example is very similar to the "minimal" example.
 *   But here we provide a status indicator LED, to get feedback
 *   of the thing state.
 *   Further more we set up a push button. If push button is detected
 *   to be pressed at boot time, the thing will use the initial
 *   password for accepting connections in Access Point mode. This
 *   is usefull e.g. when custom password was lost.
 *   (See previous examples for more details!)
 * 
 * Hardware setup for this example:
 *   - An LED is attached to LED_BUILTIN pin with setup On=LOW.
 *     This is hopefully already attached by default.
 *   - A push button is attached to pin D2, the other leg of the
 *     button should be attached to GND.
 */

#include <IoTWebConf_for_Visuino_modified_by_IoT_Jedi.h>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to build an AP. (E.g. in case of lost password)
#define CONFIG_PIN D2

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// -- Method declarations.
void handleRoot();

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  // -- Initializing the configuration.
  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ IoTWebConf_for_Visuino_modified_by_IoT_Jedi.handleConfig(); });
  server.onNotFound([](){ IoTWebConf_for_Visuino_modified_by_IoT_Jedi.handleNotFound(); });

  Serial.println("Ready.");
}

void loop() 
{
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (IoTWebConf_for_Visuino_modified_by_IoT_Jedi.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 02 Status and Reset</title></head><body>Hello world!";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

