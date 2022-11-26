#include "wlan_setup.h"

/** Unique device name */
String bluetoothDeviceName = "LED-Strip-";
/** Flag if stored AP credentials are available */
bool hasCredentials = false;
/** Connection status */
volatile bool isConnected = false;
/** Connection change status */
bool connectionStatusChanged = false;
bool bleIsRunning = false;

String ssid;
String password;
String macAddress;
BLECharacteristic *pCharacteristicWiFi;
BLEAdvertising *pBluetoothAdvertising;
BLEService *pBluetoothService;
BLEServer *pBluetoothServer;
StaticJsonDocument<100> jsonBuffer;


/**
 * Create unique device name from MAC address
 **/
void createName() {
    // Get MAC address for WiFi station
    Serial.println("Get MAC address for WiFi station");
    macAddress = WiFi.macAddress();
    Serial.println("MAC address for WiFi station received");
    bluetoothDeviceName = bluetoothDeviceName + macAddress;
}

/**
 * BluetoothCallbacks
 * Callbacks for client connection and disconnection
 */
BluetoothCallbacks::BluetoothCallbacks() = default;

void BluetoothCallbacks::onConnect(BLEServer *pServer) {
    Serial.println("BLE client connected");
}

void BluetoothCallbacks::onDisconnect(BLEServer *pServer) {
    Serial.println("BLE client disconnected");
    pBluetoothAdvertising->start();
};

/**
 * BluetoothCallbackHandler
 * Callbacks for BLE client read/write requests
 */
BluetoothCallbackHandler::BluetoothCallbackHandler() = default;

void BluetoothCallbackHandler::onWrite(BLECharacteristic *pCharacteristic) {
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

void BluetoothCallbackHandler::onRead(BLECharacteristic *pCharacteristic) {
    Serial.println("BLE onRead request");
    String credentials;

    /** Json object for outgoing data */
    StaticJsonDocument<50> jsonOut;
    jsonOut["ssid"] = ssid;
    jsonOut["password"] = password;
    // Convert JSON object into a string
    serializeJson(jsonOut, credentials);
    String ipAddress = WiFi.localIP().toString().c_str();
    pCharacteristicWiFi->setValue(ipAddress.c_str());
    jsonBuffer.clear();
}

/**
 * initBLE
 * Initialize BLE service and characteristic
 * Start BLE server and service advertising
 */
void initBLE() {
    createName();
    // Initialize BLE and set output power
    BLEDevice::init((const char *) bluetoothDeviceName.c_str());
    BLEDevice::setPower(ESP_PWR_LVL_P7);
    // Create BLE Server
    pBluetoothServer = BLEDevice::createServer();
    // Set server callbacks
    pBluetoothServer->setCallbacks(new BluetoothCallbacks());
    // Create BLE Service
    pBluetoothService = pBluetoothServer->createService(BLEUUID(SERVICE_UUID));
    // Create BLE Characteristic for WiFi settings
    pCharacteristicWiFi = pBluetoothService->createCharacteristic(
            BLEUUID(WIFI_UUID),
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::NOTIFY
    );
    pCharacteristicWiFi->setCallbacks(new BluetoothCallbackHandler());
    pCharacteristicWiFi->addDescriptor(new NimBLE2904());
    // Start the service
    pBluetoothService->start();
    // Start advertising
    pBluetoothAdvertising = pBluetoothServer->getAdvertising();
    pBluetoothAdvertising->start();
    bleIsRunning = true;
}

/**
 * Start connection to WiFi
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
    // Initialize Serial port
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

    //Start BLE server and runs all the time for possible settings changes
    if (hasCredentials) {
        connectWiFi();
        if (WiFi.localIP().toString() == "0.0.0.0") {
            //initBLE();
            waitForBluetoothConnection();
        }

    } else {
        Serial.println("No credentials found, start BLE server");
        //initBLE();
        waitForBluetoothConnection();
    }
}

void waitForBluetoothConnection() {
    connectionFailed:
    Serial.println("Waiting for Bluetooth input...");
    while (!isConnected) {
    }
    Serial.println("Connect to WiFi");
    if (!connectWiFi()) {
        isConnected = false;
        goto connectionFailed;
    }
};