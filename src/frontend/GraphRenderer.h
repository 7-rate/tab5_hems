#pragma once

#include <M5Unified.h>
#include <vector>
#include "../backend/InfluxDBManager.h"

class ConfigManager; // 前方宣言

// 時間範囲の選択肢
enum TimeRange {
    TIME_3H = 0,
    TIME_6H,
    TIME_12H,
    TIME_1D,
    TIME_3D,
    TIME_7D,
    TIME_1M
};

// Y軸スケールの選択肢
enum YAxisScale {
    SCALE_AUTO = 0,
    SCALE_500,
    SCALE_1000,
    SCALE_1500,
    SCALE_2000,
    SCALE_3000,
    SCALE_4000
};

class GraphRenderer {
private:
    int graphX, graphY, graphWidth, graphHeight;
    float minValue, maxValue;
    std::vector<DataPoint> dataPoints;
    ConfigManager* config;
    
    // スケール設定
    TimeRange currentTimeRange;
    YAxisScale currentYScale;
    
    // ボタン領域
    struct Button {
        int x, y, width, height;
        String label;
    };
    std::vector<Button> timeButtons;
    std::vector<Button> yScaleButtons;
    
    void drawAxes();
    void drawGrid();
    void drawLabels();
    void drawDataLine();
    void calculateScale();
    void drawButtons();
    int mapValueToY(float value);
    int mapTimeToX(int index);
    
public:
    GraphRenderer();
    void setConfig(ConfigManager* configManager);
    void setGraphArea(int x, int y, int width, int height);
    void setData(const std::vector<DataPoint>& data);
    void draw();
    void drawLatestValue(float value);
    void drawMonthlyEnergyUsage(float usage, bool hasData);
    void clear();
    
    // タッチ操作
    bool handleTouch(int x, int y);
    TimeRange getCurrentTimeRange() const { return currentTimeRange; }
    YAxisScale getCurrentYScale() const { return currentYScale; }
    int getTimeRangeHours() const;
    float getYScaleMax() const;
};