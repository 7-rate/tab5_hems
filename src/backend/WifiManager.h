#pragma once

#include <WiFi.h>

class WifiManager {
private:
    bool isConnected;
    
public:
    WifiManager();
    bool connect();
    bool isWifiConnected();
    void disconnect();
    String getLocalIP();
    int getSignalStrength();
};