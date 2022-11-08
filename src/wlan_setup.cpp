#include "wlan_setup.h"

/** Unique device name */
char apName[] = "LED-Strip";
/** Flag if stored AP credentials are available */
bool hasCredentials = false;
/** Connection status */
volatile bool isConnected = false;
/** Connection change status */
bool connectionStatusChanged = false;
bool bleIsRunning = false;

String ssid;
String password;
BLECharacteristic *pCharacteristicWiFi;
BLEAdvertising *pAdvertising;
BLEService *pService;
BLEServer *pServer;
StaticJsonDocument<100> jsonBuffer;


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

/**
 * MyServerCallbacks
 * Callbacks for client connection and disconnection
 */
MyServerCallbacks::MyServerCallbacks() {
}

void MyServerCallbacks::onConnect(BLEServer *pServer) {
    Serial.println("BLE client connected");
}

void MyServerCallbacks::onDisconnect(BLEServer *pServer) {
    Serial.println("BLE client disconnected");
    pAdvertising->start();
};

/**
 * MyCallbackHandler
 * Callbacks for BLE client read/write requests
 */
MyCallbackHandler::MyCallbackHandler() {
}

void MyCallbackHandler::onWrite(BLECharacteristic *pCharacteristic) {
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
    hasCredentials = true;
    connectionStatusChanged = true;
    isConnected = true;
    jsonBuffer.clear();
}

void MyCallbackHandler::onRead(BLECharacteristic *pCharacteristic) {
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

/**
 * initBLE
 * Initialize BLE service and characteristic
 * Start BLE server and service advertising
 */
void initBLE() {
    // Initialize BLE and set output power
    Serial.println("Initializing BLE...");
    BLEDevice::init(apName);
    Serial.println("BLE initialized");
    BLEDevice::setPower(ESP_PWR_LVL_P7);

    // Create BLE Server
    pServer = BLEDevice::createServer();

    // Set server callbacks
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE Service
    pService = pServer->createService(BLEUUID(SERVICE_UUID));

    // Create BLE Characteristic for WiFi settings
    pCharacteristicWiFi = pService->createCharacteristic(
            BLEUUID(WIFI_UUID),
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::NOTIFY
    );
    pCharacteristicWiFi->setCallbacks(new MyCallbackHandler());

    pCharacteristicWiFi->addDescriptor(new NimBLE2904());
    // Start the service
    pService->start();

    // Start advertising
    pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
    bleIsRunning = true;
}

/**
 * Start connection to AP
 */
bool connectWiFi() {
    WiFi.disconnect(true);
    WiFi.enableSTA(true);
    WiFi.mode(WIFI_STA);

    Serial.println();
    Serial.print("Start connection to ");
    Serial.println(ssid);

    WiFi.begin(ssid.c_str(), password.c_str());
    delay(500);
    Serial.println("Wait for connection");
    while (WiFi.status() != WL_CONNECTED) {
        if (WiFi.status() == WL_NO_SSID_AVAIL) {
            Serial.println("Connection failed. Wrong SSID");
            hasCredentials = false;
            connectionStatusChanged = false;
            if (bleIsRunning) {
                String ipAdress = WiFi.localIP().toString();
                std::string stdSTring = (ipAdress.c_str());
                pCharacteristicWiFi->setValue(stdSTring);
                pCharacteristicWiFi->notify();
            }
            return false;
        }
        if (WiFi.status() == WL_CONNECT_FAILED) {
            Serial.println("Connection failed.");
            hasCredentials = false;
            connectionStatusChanged = false;
            if (bleIsRunning) {
                String ipAdress = WiFi.localIP().toString();
                std::string stdSTring = (ipAdress.c_str());
                pCharacteristicWiFi->setValue(stdSTring);
                pCharacteristicWiFi->notify();
            }
            return false;
        }
    }
    Serial.println("");
    Serial.println("Connection established!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    if (bleIsRunning) {
        String ipAdress = WiFi.localIP().toString();
        std::string stdSTring = (ipAdress.c_str());
        pCharacteristicWiFi->setValue(stdSTring);
        pCharacteristicWiFi->notify();
    }
    return true;
}

void setupWlan() {
    // Create unique device name
    createName();
    // Initialize Serial port
    Serial.begin(115200);

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

    // Start BLE server and runs all the time for possible settings changes
//    if (hasCredentials) {
//        connectWiFi();
//
//        if (WiFi.localIP().toString() == "0.0.0.0") {
//            initBLE();
//            waitForBluetoothConnection();
//        }
//
//    } else {
//        Serial.println("No credentials found, start BLE server");
//        initBLE();
//        waitForBluetoothConnection();
//    }
    initBLE();
    //waitForBluetoothConnection();
}

void waitForBluetoothConnection() {
    connectionFailed:
    Serial.println("Waiting for Bluetooth input");
    while (!isConnected) {
    }
    Serial.println("Connect to WiFi");
    if (!connectWiFi()) {
        isConnected = false;
        goto connectionFailed;
    }
};