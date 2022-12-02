#include <Arduino.h>
#include "wlan.h"
#include "api.h"
#include "led.h"

#define TIMER_MS 400

unsigned long last_change = 0;
unsigned long now = 0;
Led led;
Led *ledPointer = &led;
Api api(ledPointer);

void setup() {
    Serial.begin(115200);
    Bluetooth::bluetoothInit();
    wlanInit();
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);
    delay(10);
    //led.startLed();
    api.startServer();
    Serial.println("Setup done");
}

void loop() {
    api.serverHandleClient();
    //led.ledService();
    if (millis() - last_change > TIMER_MS) {
        last_change = now;
    }
}
