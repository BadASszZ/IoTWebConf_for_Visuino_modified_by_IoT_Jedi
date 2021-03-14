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

#ifndef IoTWebConf_for_Visuino_modified_by_IoT_JediUsing_h
#define IoTWebConf_for_Visuino_modified_by_IoT_JediUsing_h

// This "using" lines are just aliases, and should avoided.

using IotWebConfParameterGroup = iotwebconf::ParameterGroup;
using IotWebConfTextParameter = iotwebconf::TextParameter;
using IotWebConfPasswordParameter = iotwebconf::PasswordParameter;
using IotWebConfNumberParameter = iotwebconf::NumberParameter;
using IotWebConfCheckboxParameter = iotwebconf::CheckboxParameter;
using IotWebConfSelectParameter = iotwebconf::SelectParameter;

#endif