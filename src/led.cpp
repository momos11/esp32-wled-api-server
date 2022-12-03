#include "led.h"
#include <WS2812FX.h>
#include "Preferences.h"

Led::Led() : ws2812fx(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800) {
}

void Led::startLed() {
    Preferences preferences;
    preferences.begin("state", false);
    Serial.println("Try");
    try {
        uint32_t primaryColor = preferences.getUInt("primaryColor", 0x007BFF);
        uint32_t secondaryColor = preferences.getUInt("secondaryColor", 0x007BFF);
        uint32_t colors[] = {primaryColor, secondaryColor, 0x00000};
        uint32_t ledMode = preferences.getUInt("ledMode", 0);
        uint32_t speed = preferences.getUInt("speed", 0);
        uint32_t brightness = preferences.getUInt("brightness", 0);
        ws2812fx.setMode(ledMode);
        ws2812fx.setSpeed(100);
        ws2812fx.setColors(0, colors);
        //ws2812fx.setBrightness(100);
    }
    catch (...) {
        Serial.println("Error while loading preferences");
    }
    Serial.println("Settings loaded");
    ws2812fx.init();
    Serial.println("Led init");

    if (preferences.getBool("toggleState", true)) {
        ws2812fx.start();
        Serial.println("Led started");
    } else {
        ws2812fx.stop();
        Serial.println("Led stopped");
    }
    preferences.end();
}

void Led::ledService() {
    ws2812fx.service();
}
