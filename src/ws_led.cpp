#include "ws_led.h"
#include "WS2812FX.h"
#include "Preferences.h"

Led::Led() : ws2812fx(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800) {
}

void Led::startLed() {
    Preferences preferences;

    preferences.begin("state", false);

    ws2812fx.setMode(preferences.getInt("ledMode", 0));
    ws2812fx.setSpeed(preferences.getInt("speed", 2000));
    ws2812fx.setColor(preferences.getInt("primaryColor", 0x007BFF));
    ws2812fx.setColor(preferences.getInt("secondaryColor", 0x007BFF));
    ws2812fx.setBrightness(preferences.getInt("brightness", 255));

    ws2812fx.init();

    if (preferences.getBool("toggleState"), true) {
        ws2812fx.start();
    }
    preferences.end();
}

void Led::ledService() {
    ws2812fx.service();
}
