//
// Created by Maurice Golob on 23.01.22.
//

#ifndef ESP32_WLED_API_SERVER_UPDATE_ESP_H
#define ESP32_WLED_API_SERVER_UPDATE_ESP_H

#include <cstdint>
#include <cstddef>


void updateInit();

void update_esp();

void updateFirmware(uint8_t *data, size_t len);

#endif //ESP32_WLED_API_SERVER_UPDATE_ESP_H
