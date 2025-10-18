#pragma once

#include <vector>
#include <Arduino.h>
#include <M5Unified.h>

#define SDIO2_CLK GPIO_NUM_12
#define SDIO2_CMD GPIO_NUM_13
#define SDIO2_D0  GPIO_NUM_11
#define SDIO2_D1  GPIO_NUM_10
#define SDIO2_D2  GPIO_NUM_9
#define SDIO2_D3  GPIO_NUM_8
#define SDIO2_RST GPIO_NUM_15

// データソース設定構造体
struct DataSourceConfig {
    String measurement;
    String field;
    String unit;
    String displayName;
    uint32_t color;
    float minRange;
    float maxRange;
    bool autoScale;
};

// グラフ設定構造体
struct GraphConfig {
    String title;
    String xAxisLabel;
    String yAxisLabel;
    int graphX;
    int graphY;
    int graphWidth;
    int graphHeight;
    int gridLines;
    bool showGrid;
    bool showLegend;
};

// システム設定構造体
struct SystemConfig {
    int updateIntervalMinutes;
    int dataHours;
    bool enableWiFiReconnect;
    int reconnectTimeoutSeconds;
    bool enableSerial;
    bool enableStatusDisplay;
};

class ConfigManager {
private:
    DataSourceConfig dataSource;
    GraphConfig graph;
    SystemConfig system;
    
public:
    ConfigManager();
    
    // 設定の取得
    const DataSourceConfig& getDataSourceConfig() const { return dataSource; }
    const GraphConfig& getGraphConfig() const { return graph; }
    const SystemConfig& getSystemConfig() const { return system; }
    
    // 設定の変更
    void setDataSource(const String& measurement, const String& field, const String& unit = "units");
    void setGraphTitle(const String& title);
    void setGraphAxes(const String& xLabel, const String& yLabel);
    void setUpdateInterval(int minutes);
    void setDataRange(int hours);
    void setGraphArea(int x, int y, int width, int height);
    void setAutoScale(bool enable);
    void setValueRange(float min, float max);
    
    // プリセット設定
    void loadTemperatureConfig();
    void loadHumidityConfig();
    void loadPowerConfig();
    void loadCustomConfig(const String& measurement, const String& field);
};