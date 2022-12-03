#include <Arduino.h>
#include "wlan.h"
#include "api.h"
#include "led.h"

#define TIMER_MS 25

unsigned long last_change = 0;
unsigned long now = 0;
Led led;
Led *p_led = &led;
Api api(p_led);

void setup() {
    Serial.begin(115200);
    Bluetooth::bluetoothInit();
    Wlan::wlanInit();
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);
    delay(10);
    led.startLed();
    api.startServer();
}

void loop() {
    api.serverHandleClient();
    led.ledService();
    if (millis() - last_change > TIMER_MS) {
        last_change = now;
    }
}
