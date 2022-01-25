#include "ws_led.h"
#include <WS2812FX.h>
#include "Preferences.h"

Led::Led() : ws2812fx(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800) {
}

void Led::startLed() {
    Preferences preferences;
    preferences.begin("state", false);
    uint32_t primaryColor = preferences.getUInt("primaryColor", 0x007BFF);
    uint32_t secondaryColor = preferences.getUInt("secondaryColor", 0x007BFF);
    uint32_t colors[] = {primaryColor, secondaryColor, 0x00000};
    ws2812fx.setMode(preferences.getUInt("ledMode", 0));
    ws2812fx.setSpeed(preferences.getUInt("speed", 2000));
    ws2812fx.setColors(0, colors);
    ws2812fx.setBrightness(preferences.getUInt("brightness", 255));
    ws2812fx.init();

    if (preferences.getBool("toggleState", true)) {
        ws2812fx.start();
    } else {
        ws2812fx.stop();
    }
    preferences.end();
}

void Led::ledService() {
    ws2812fx.service();
}
