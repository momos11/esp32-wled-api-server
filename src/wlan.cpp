#include "wlan.h"

String ssid;
String password;

bool Wlan::checkForCredentials() {
    Preferences preferences;
    preferences.begin("WiFiCred", false);
    bool hasPreference = preferences.getBool("valid", false);
    if (hasPreference) {
        ssid = preferences.getString("ssid", "");
        password = preferences.getString("password", "");
        if (ssid.equals("") || password.equals("")) {
            Serial.println("Found preferences but credentials are empty");
            preferences.end();
            return false;
        } else {
            Serial.println("Read from preferences:");
            Serial.println("primary SSID: " + ssid + " password: " + password);
        }
    } else {
        Serial.println("Could not find preferences, need send data over BLE");
    }
    preferences.end();
    return hasPreference;
}

void Wlan::wlanInit() {
    Serial.println("Setup WLAN");
    if (checkForCredentials()) {
        Serial.println("Got credentials from preferences");
        connectToWiFi();
        if (WiFi.localIP().toString() == "0.0.0.0") {
            waitForBluetoothConnection();
        }
    } else {
        Serial.println("No credentials found, waiting for bluetooth connection");
        waitForBluetoothConnection();
    }
}

/**
 * Start connection to WiFi
 */
bool Wlan::connectToWiFi() {
    WiFi.disconnect(true);
    WiFi.enableSTA(true);
    WiFiClass::mode(WIFI_STA);

    Serial.println();
    Serial.print("Start connection to ");
    Serial.println(ssid);

    WiFi.begin(ssid.c_str(), password.c_str());
    delay(500);

    Serial.println("Wait for connection");
    while (WiFiClass::status() != WL_CONNECTED) {
        if (WiFiClass::status() == WL_NO_SSID_AVAIL) {
            Serial.println("Connection failed. Wrong SSID");
            return false;
        }
        if (WiFiClass::status() == WL_CONNECT_FAILED) {
            Serial.println("Connection failed.");
            return false;
        }
    }
    Serial.println("");
    Serial.println("Connection established!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    String ipAddress = WiFi.localIP().toString();

    Bluetooth::setWifiCharacteristicValue(ipAddress.c_str());
    Bluetooth::notifyWifiCharacteristic();

    return true;
}

void Wlan::waitForBluetoothConnection() {
    Serial.println("Waiting for Bluetooth input...");
    while (!Bluetooth::getDataReceived()) {
        delay(100);
    }
    Serial.println("Got Bluetooth input");
    Wlan::connectToWiFi();
};