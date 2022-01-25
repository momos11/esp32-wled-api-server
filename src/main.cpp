#include <Arduino.h>
#include "wlan_setup.h"
#include "api.h"
#include "ws_led.h"
#include "update_esp.h"

#define TIMER_MS 400

unsigned long last_change = 0;
unsigned long now = 0;
Led led;
Led *ledPointer = &led;
Api api(ledPointer);

void setup() {
    //connects to WLAN if credentials are stored
    setupWlan();
    //update_esp();
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);
    delay(10);
    led.startLed();
    api.startServer();
    Serial.println("LED started");
}

void loop() {
    api.serverHandleClient();
    led.ledService();
    if (millis() - last_change > TIMER_MS) {
        last_change = now;
    }
}
