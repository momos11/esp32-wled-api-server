#include <Arduino.h>
#include "wlan.h"
#include "api.h"
#include "led.h"

#define TIMER_MS 400

unsigned long last_change = 0;
unsigned long now = 0;
Led *p_led = new Led();
Api api(p_led);

void setup() {
    Serial.begin(115200);
    Bluetooth::bluetoothInit();
    Wlan::wlanInit();
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);
    delay(10);
    p_led->startLed();
    api.startServer();
    Serial.println("Setup done");
}

void loop() {
    api.serverHandleClient();
    //p_led.ledService();
    if (millis() - last_change > TIMER_MS) {
        last_change = now;
    }
}
