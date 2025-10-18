#include "WifiManager.h"
#include "../../include/env.h"

WifiManager::WifiManager() : isConnected(false) {}

bool WifiManager::connect() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.setPins(GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_11, GPIO_NUM_10, GPIO_NUM_9, GPIO_NUM_8,
                 GPIO_NUM_15);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    const int maxAttempts = 30; // 30 seconds timeout

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        isConnected = true;
        Serial.println();
        Serial.println("Wi-Fi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        isConnected = false;
        Serial.println();
        Serial.println("Wi-Fi connection failed!");
        return false;
    }
}

bool WifiManager::isWifiConnected() { return WiFi.status() == WL_CONNECTED && isConnected; }

void WifiManager::disconnect() {
    WiFi.disconnect();
    isConnected = false;
    Serial.println("Wi-Fi disconnected");
}

String WifiManager::getLocalIP() {
    if (isWifiConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not connected";
}

int WifiManager::getSignalStrength() {
    if (isWifiConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}