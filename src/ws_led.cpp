#include "ws_led.h"
#include "WS2812FX.h"

Led::Led() : ws2812fx(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800) {



}

void Led::startLed() {
    ws2812fx.setBrightness(255);
    ws2812fx.setSpeed(2000);
    uint32_t colors[] = {0xec16c4,0xfee819,0x00000};
    ws2812fx.setColors(0,colors);
    ws2812fx.setMode(FX_MODE_CUSTOM_1);
    ws2812fx.init();
    ws2812fx.start();
}

void Led::ledService() {
    ws2812fx.service();
}
