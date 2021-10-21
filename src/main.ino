// Default Arduino includes
#include <Arduino.h>
#include <WiFi.h>
#include <nvs.h>
#include <nvs_flash.h>

// Includes for JSON object handling
// Requires ArduinoJson library
// https://arduinojson.org
// https://github.com/bblanchon/ArduinoJson
#include <ArduinoJson.h>
#include <stdlib.h>
#include <string.h>
#include <base64encode.h>
// Includes for BLE
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <Preferences.h>

/** Build time */
const char compileDate[] = __DATE__ " " __TIME__;

/** Unique device name */
char apName[] = "LED-Strip";
/** Selected network
    true = use primary network
		false = use secondary network
*/
bool usePrimAP = true;
/** Flag if stored AP credentials are available */
bool hasCredentials = false;
/** Connection status */
volatile bool isConnected = false;
/** Connection change status */
bool connectionStatusChanged = false;

/**
 * Create unique device name from MAC address
 **/
void createName() {
    uint8_t baseMac[6];
    // Get MAC address for WiFi station
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    // Write unique name into apName
    sprintf(apName, "LED-Strip-%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4],
            baseMac[5]);
}

// List of Service and Characteristic UUIDs
#define SERVICE_UUID  "f9c521f6-0f14-4499-8f76-43116b40007d"
#define WIFI_UUID     "23456f8d-4aa7-4a61-956b-39c9bce0ff00"

/** SSIDs of local WiFi networks */
String ssid;
/** Password for local WiFi network */
String password;

/** Characteristic for digital output */
BLECharacteristic *pCharacteristicWiFi;
/** BLE Advertiser */
BLEAdvertising *pAdvertising;
/** BLE Service */
BLEService *pService;
/** BLE Server */
BLEServer *pServer;

/** Buffer for JSON string */
// MAx size is 51 bytes for frame:
// {"ssid":"","password":"","ssidSec":"","pwSec":""}
// + 4 x 32 bytes for 2 SSID's and 2 passwords
StaticJsonDocument<100> jsonBuffer;

/**
 * MyServerCallbacks
 * Callbacks for client connection and disconnection
 */
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        Serial.println("BLE client connected");
    };

    void onDisconnect(BLEServer *pServer) {
        Serial.println("BLE client disconnected");
        pAdvertising->start();
    }
};

/**
 * MyCallbackHandler
 * Callbacks for BLE client read/write requests
 */
class MyCallbackHandler : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() == 0) {
            return;
        }
        Serial.println("Received over BLE: " + String((char *) &value[0]));

        /** Json object for incoming data */
        StaticJsonDocument<256> jsonIn;
        deserializeJson(jsonIn, String((char *) &value[0]));
        ssid = jsonIn["ssid"].as<String>();
        password = jsonIn["password"].as<String>();

        Preferences preferences;
        preferences.begin("WiFiCred", false);
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.putBool("valid", true);
        preferences.end();

        Serial.println("Received over bluetooth:");
        Serial.println("primary SSID: " + ssid + " password: " + password);
        connectionStatusChanged = true;
        hasCredentials = true;
//        } else if (jsonIn.containsKey("erase")) {
//            Serial.println("Received erase command");
//            Preferences preferences;
//            preferences.begin("WiFiCred", false);
//            preferences.clear();
//            preferences.end();
//            connectionStatusChanged = true;
//            hasCredentials = false;
//            ssid = "";
//            password = "";
//
//            int err;
//            err = nvs_flash_init();
//            Serial.println(&"nvs_flash_init: "[err]);
//            err = nvs_flash_erase();
//            Serial.println(&"nvs_flash_erase: "[err]);
//        } else if (jsonIn.containsKey("reset")) {
//            WiFi.disconnect();
//            esp_restart();
//        }

        jsonBuffer.clear();
    };

    void onRead(BLECharacteristic *pCharacteristic) {
        Serial.println("BLE onRead request");
        String credentials;

        /** Json object for outgoing data */
        StaticJsonDocument<50> jsonOut;
        jsonOut["ssid"] = ssid;
        jsonOut["password"] = password;
        // Convert JSON object into a string
        serializeJson(jsonOut, credentials);

        String ipAdress = WiFi.localIP().toString();
        std::string stdSTring = (ipAdress.c_str());
        pCharacteristicWiFi->setValue(stdSTring);
        jsonBuffer.clear();
    }
};

/**
 * initBLE
 * Initialize BLE service and characteristic
 * Start BLE server and service advertising
 */
void initBLE() {
    // Initialize BLE and set output power
    BLEDevice::init(apName);
    BLEDevice::setPower(ESP_PWR_LVL_P7);

    // Create BLE Server
    pServer = BLEDevice::createServer();

    // Set server callbacks
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE Service
    pService = pServer->createService(BLEUUID(SERVICE_UUID), 20);

    // Create BLE Characteristic for WiFi settings
    pCharacteristicWiFi = pService->createCharacteristic(
            BLEUUID(WIFI_UUID),
            // WIFI_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristicWiFi->setCallbacks(new MyCallbackHandler());

    // Start the service
    pService->start();

    // Start advertising
    pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}

/** Callback for receiving IP address from AP */
void gotIP(system_event_id_t event) {
    isConnected = true;
    connectionStatusChanged = true;
}

/** Callback for connection loss */
void lostCon(system_event_id_t event) {
    isConnected = false;
    connectionStatusChanged = true;
}

/**
 * Start connection to AP
 */
void connectWiFi() {
    // Setup callback function for successful connection
    WiFi.onEvent(gotIP, SYSTEM_EVENT_STA_GOT_IP);
    // Setup callback function for lost connection
    WiFi.onEvent(lostCon, SYSTEM_EVENT_STA_DISCONNECTED);

    WiFi.disconnect(true);
    WiFi.enableSTA(true);
    WiFi.mode(WIFI_STA);

    Serial.println();
    Serial.print("Start connection to ");
    if (usePrimAP) {
        Serial.println(ssid);
        WiFi.begin(ssid.c_str(), password.c_str());
    }
}

void setup() {
    // Create unique device name
    createName();

    // Initialize Serial port
    Serial.begin(115200);
    // Send some device info
    Serial.print("Build: ");
    Serial.println(compileDate);

    Preferences preferences;
    preferences.begin("WiFiCred", false);
    bool hasPref = preferences.getBool("valid", false);
    if (hasPref) {
        ssid = preferences.getString("ssid", "");
        password = preferences.getString("password", "");
        if (ssid.equals("")
            || password.equals("")) {
            Serial.println("Found preferences but credentials are invalid");
        } else {
            Serial.println("Read from preferences:");
            Serial.println("primary SSID: " + ssid + " password: " + password);
            hasCredentials = true;
        }
    } else {
        Serial.println("Could not find preferences, need send data over BLE");
    }
    preferences.end();

    // Start BLE server
    initBLE();

    if (hasCredentials) {
        // Check for available AP's
        // If AP was found, start connection
        connectWiFi();
    }
}

void loop() {
    if (connectionStatusChanged) {
        if (isConnected) {
            Serial.print("Connected to AP: ");
            Serial.print(WiFi.SSID());
            Serial.print(" with IP: ");
            Serial.print(WiFi.localIP());
            Serial.print(" RSSI: ");
            Serial.println(WiFi.RSSI());
        } else {
            if (hasCredentials) {
                Serial.println("Lost WiFi connection");
                // Received WiFi credentials
                // If AP was found, start connection
                connectWiFi();
            }
        }
        connectionStatusChanged = false;
    }
}