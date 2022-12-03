#include "bluetooth.h"

String bluetoothDeviceName = "LED-Strip-";

StaticJsonDocument<100> jsonBuffer;

void Bluetooth::bluetoothInit() {
    Serial.println("Setup Bluetooth");
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
    // Create BLE Characteristic for Wi-Fi settings
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
}

/**
 * BluetoothCallbacks
 * Callbacks for client connection and disconnection
 */
Bluetooth::BluetoothCallbacks::BluetoothCallbacks() = default;

void Bluetooth::BluetoothCallbacks::onConnect(BLEServer *pServer) {
    Serial.println("BLE client connected");
}


void Bluetooth::BluetoothCallbacks::onDisconnect(BLEServer *pServer) {
    Serial.println("BLE client disconnected");
    pBluetoothAdvertising->start();
}

/**
 * BluetoothCallbackHandler
 * Callbacks for BLE client read/write requests
 */
Bluetooth::BluetoothCallbackHandler::BluetoothCallbackHandler() = default;

void Bluetooth::BluetoothCallbackHandler::onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 0) {
        return;
    }
    Serial.println("Received over BLE: " + String((char *) &value[0]));

    /** Json object for incoming data */
    StaticJsonDocument<256> jsonIn;
    deserializeJson(jsonIn, String((char *) &value[0]));
    m_ssid = jsonIn["ssid"].as<String>();
    m_password = jsonIn["password"].as<String>();

    Preferences preferences;
    preferences.begin("WiFiCred", false);
    preferences.putString("ssid", m_ssid);
    preferences.putString("password", m_password);
    preferences.putBool("valid", true);
    preferences.end();

    Serial.println("Received over bluetooth:");
    Serial.println("primary SSID: " + m_ssid + " password: " + m_password);

    jsonBuffer.clear();

    setDataReceived(true);
}

void Bluetooth::BluetoothCallbackHandler::onRead(BLECharacteristic *pCharacteristic) {
    Serial.println("BLE onRead request");
    String credentials;

    /** Json object for outgoing data */
    StaticJsonDocument<50> jsonOut;
    jsonOut["ssid"] = m_ssid;
    jsonOut["password"] = m_password;
    // Convert JSON object into a string
    serializeJson(jsonOut, credentials);
    String ipAddress = WiFi.localIP().toString().c_str();
    pCharacteristicWiFi->setValue(ipAddress.c_str());
    jsonBuffer.clear();
}

/**
 * Create unique device name from MAC address
 **/
void Bluetooth::createName() {
    // Get MAC address for Wi-Fi station
    Serial.println("Get MAC address for WiFi station");
    m_macAddress = WiFi.macAddress();
    Serial.println("MAC address for WiFi station received");
    bluetoothDeviceName = bluetoothDeviceName + m_macAddress;
}

void Bluetooth::stopAdvertising() {
    pBluetoothAdvertising->stop();
}

bool Bluetooth::getDataReceived() {
    return dataReceived;
}

void Bluetooth::setDataReceived(bool received) {
    dataReceived = received;
}

void Bluetooth::setWifiCharacteristicValue(String value) {
    pCharacteristicWiFi->setValue(value.c_str());
}

void Bluetooth::notifyWifiCharacteristic() {
    pCharacteristicWiFi->notify();
}


