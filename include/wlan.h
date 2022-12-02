//
// Created by mauri on 14.12.2021.
//

#ifndef ESP32_WLED_API_SERVER_WLAN_H
#define ESP32_WLED_API_SERVER_WLAN_H

#include <WiFi.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "ArduinoJson.h"
#include <Preferences.h>
#include "bluetooth.h"

bool connectToWiFi();

void wlanInit();

void waitForBluetoothConnection();


#endif //ESP32_WLED_API_SERVER_WLAN_H
