#ifndef ESP32_WLED_API_SERVER_LED_H
#define ESP32_WLED_API_SERVER_LED_H

#include <WS2812FX.h>

#define LEDPIN 12
#define NUMPIXELS 120

class Led {
public:
    WS2812FX ws2812fx;

    Led();

    void startLed();

    void ledService();
};


#endif //ESP32_WLED_API_SERVER_LED_H
