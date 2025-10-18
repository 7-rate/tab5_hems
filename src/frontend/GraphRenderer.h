#pragma once

#include <M5Unified.h>
#include <vector>
#include "../backend/InfluxDBManager.h"

class ConfigManager; // 前方宣言

class GraphRenderer {
private:
    int graphX, graphY, graphWidth, graphHeight;
    float minValue, maxValue;
    std::vector<DataPoint> dataPoints;
    ConfigManager* config;
    
    void drawAxes();
    void drawGrid();
    void drawLabels();
    void drawDataLine();
    void calculateScale();
    int mapValueToY(float value);
    int mapTimeToX(int index);
    
public:
    GraphRenderer();
    void setConfig(ConfigManager* configManager);
    void setGraphArea(int x, int y, int width, int height);
    void setData(const std::vector<DataPoint>& data);
    void draw();
    void drawLatestValue(float value);
    void clear();
};