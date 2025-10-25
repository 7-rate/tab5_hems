#pragma once

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <vector>

struct DataPoint {
    String timestamp;
    float value;
};

class ConfigManager; // 前方宣言

class InfluxDBManager {
private:
    ::InfluxDBClient* client;
    ConfigManager* config;
    String buildFluxQuery();
    
public:
    InfluxDBManager();
    ~InfluxDBManager();
    void setConfig(ConfigManager* configManager);
    bool connect();
    std::vector<DataPoint> getData();
    float getLatestValue();
    bool getMonthlyEnergyUsage(float &monthlyUsage);
    bool isConnected();
};