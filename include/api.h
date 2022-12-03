#ifndef ESP32_WLED_API_SERVER_API_H
#define ESP32_WLED_API_SERVER_API_H

#include "WS2812FX.h"
#include "led.h"
#include "WebServer.h"
#include <ArduinoJson.h>

class Api {
public:
    Led *led;
    WebServer server;
    uint32_t primaryColor;
    uint32_t secondaryColor;
    uint32_t colors[3]{};
    bool toggleState;
    int ledMode;
    int speed;
    int brightness;

    explicit Api(Led *led);

    void handleBrightness();

    void handleLedMode();

    void setCrossOrigin();

    void handleSpeed();

    void handleColor();

    void handleToggle();

    void handleToggleGet();

    void apiInit();

    void startServer();

    void serverHandleClient();

    void handleBrightnessGet();

    void handleLedModeGet();

    void handleInformationGet();

    void handleColorGet();

    static void saveSettings(int key, uint32_t value);

    static void setupOTA();

    void handleReset();

    void handleUpdate();
};

#endif //ESP32_WLED_API_SERVER_API_H
