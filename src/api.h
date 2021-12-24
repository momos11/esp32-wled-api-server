#ifndef ESP32_WLED_API_SERVER_API_H
#define ESP32_WLED_API_SERVER_API_H

#include "WS2812FX.h"
#include "ws_led.h"
#include "WebServer.h"

class Api {
public:
    Led *led;
    WebServer server;
    String colorString;
    String side;
    String colorLeft;
    String colorRight;
    String toggleState;
    int ledMode;
    int speed;
    int brightness;

    Api(Led *led);

    void handleBrightness();

    void handleMode();

    void setCrossOrigin();

    void handleSpeed();

    void handleColor();

    void handleRightColor();

    void handleToggle();

    void handleToggleGet();

    void apiInit();

    void startServer();

    void serverHandleClient();

    void handleBrightnessGet();

    void handleModeGet();

    void handleInformationGet();
};

#endif //ESP32_WLED_API_SERVER_API_H
