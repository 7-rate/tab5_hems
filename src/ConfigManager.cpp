#include "../include/ConfigManager.h"
#include "../include/env.h"
#include <M5GFX.h>

ConfigManager::ConfigManager() {
    // デフォルト設定
    dataSource.measurement = MEASUREMENT_NAME;
    dataSource.field = FIELD_NAME_INSTANT_POWER_W;
    dataSource.unit = "W";
    dataSource.displayName = "Sensor Data";
    dataSource.color = TFT_YELLOW;
    dataSource.minRange = 0.0;
    dataSource.maxRange = 100.0;
    dataSource.autoScale = true;
    
    graph.title = GRAPH_TITLE;
    graph.xAxisLabel = X_AXIS_LABEL;
    graph.yAxisLabel = Y_AXIS_LABEL;
    graph.graphX = 100;
    graph.graphY = 120;
    graph.graphWidth = 1080;
    graph.graphHeight = 380;
    graph.gridLines = 8;
    graph.showGrid = true;
    graph.showLegend = true;
    
    system.updateIntervalMinutes = DATA_INTERVAL_MINUTES;
    system.dataHours = DATA_HOURS;
    system.enableWiFiReconnect = true;
    system.reconnectTimeoutSeconds = 30;
    system.enableSerial = true;
    system.enableStatusDisplay = true;
}

void ConfigManager::setDataSource(const String& measurement, const String& field, const String& unit) {
    dataSource.measurement = measurement;
    dataSource.field = field;
    dataSource.unit = unit;
    dataSource.displayName = field + " (" + unit + ")";
}

void ConfigManager::setGraphTitle(const String& title) {
    graph.title = title;
}

void ConfigManager::setGraphAxes(const String& xLabel, const String& yLabel) {
    graph.xAxisLabel = xLabel;
    graph.yAxisLabel = yLabel;
}

void ConfigManager::setUpdateInterval(int minutes) {
    system.updateIntervalMinutes = minutes;
}

void ConfigManager::setDataRange(int hours) {
    system.dataHours = hours;
}

void ConfigManager::setGraphArea(int x, int y, int width, int height) {
    graph.graphX = x;
    graph.graphY = y;
    graph.graphWidth = width;
    graph.graphHeight = height;
}

void ConfigManager::setAutoScale(bool enable) {
    dataSource.autoScale = enable;
}

void ConfigManager::setValueRange(float min, float max) {
    dataSource.minRange = min;
    dataSource.maxRange = max;
    dataSource.autoScale = false;
}

void ConfigManager::loadCustomConfig(const String& measurement, const String& field) {
    setDataSource(measurement, field);
    setGraphTitle(field + " Monitor");
    setGraphAxes("Time", field);
    setAutoScale(true);
    dataSource.color = TFT_YELLOW;
}