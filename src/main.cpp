#include <Arduino.h>
#include "wlan_setup.h"
#include "api.h"
#include "ws_led.h"

#define TIMER_MS 500

unsigned long last_change = 0;
unsigned long now = 0;
Led led;
Led *ledPointer = &led;
Api api(ledPointer);

void setup() {
    //connects to WLAN if credentials are stored
    setupWlan();
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);
    delay(10);
    api.startServer();
    led.startLed();
    Serial.println("LED started");
}

void loop() {
    api.serverHandleClient();
    led.ledService();
    if (millis() - last_change > TIMER_MS) {
        last_change = now;
    }
}
