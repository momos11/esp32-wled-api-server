#include "ws_led.h"
#include "WS2812FX.h"

Led::Led() : ws2812fx(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800) {



}

void Led::startLed() {
    ws2812fx.setBrightness(255);
    ws2812fx.setSpeed(2000);
    ws2812fx.setColor(0,0x007BFF);
    ws2812fx.setColor(1,0x007BFF);
    ws2812fx.setMode(FX_MODE_STATIC);
    ws2812fx.init();
    ws2812fx.start();
}

void Led::ledService() {
    ws2812fx.service();
}
