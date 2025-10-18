#include "GraphRenderer.h"
#include "../../include/env.h"
#include "../../include/ConfigManager.h"

GraphRenderer::GraphRenderer() : config(nullptr) {
    // デフォルトのグラフエリアを設定（1280x720の横レイアウト想定）
    setGraphArea(100, 80, 900, 500);
    minValue = 0;
    maxValue = 100;
}

void GraphRenderer::setConfig(ConfigManager *configManager) {
    config = configManager;
    if (config) {
        const auto &graphConfig = config->getGraphConfig();
        setGraphArea(graphConfig.graphX, graphConfig.graphY, graphConfig.graphWidth,
                     graphConfig.graphHeight);
    }
}

void GraphRenderer::setGraphArea(int x, int y, int width, int height) {
    graphX = x;
    graphY = y;
    graphWidth = width;
    graphHeight = height;
}

void GraphRenderer::setData(const std::vector<DataPoint> &data) {
    dataPoints = data;
    calculateScale();
}

void GraphRenderer::calculateScale() {
    if (dataPoints.empty()) {
        minValue = 0;
        maxValue = 100;
        return;
    }

    // ConfigManagerが設定されている場合は、その設定を使用
    if (config) {
        const auto &dataConfig = config->getDataSourceConfig();
        if (!dataConfig.autoScale) {
            minValue = dataConfig.minRange;
            maxValue = dataConfig.maxRange;
            return;
        }
    }

    // 自動スケーリング
    minValue = dataPoints[0].value;
    maxValue = dataPoints[0].value;

    for (const auto &point : dataPoints) {
        if (point.value < minValue)
            minValue = point.value;
        if (point.value > maxValue)
            maxValue = point.value;
    }

    // 余白を追加
    float range = maxValue - minValue;
    if (range == 0)
        range = 1; // ゼロ除算を防ぐ

    minValue -= range * 0.1;
    maxValue += range * 0.1;
}

int GraphRenderer::mapValueToY(float value) {
    if (maxValue == minValue)
        return graphY + graphHeight / 2;

    float ratio = (value - minValue) / (maxValue - minValue);
    return graphY + graphHeight - (int)(ratio * graphHeight);
}

int GraphRenderer::mapTimeToX(int index) {
    if (dataPoints.size() <= 1)
        return graphX;

    return graphX + (index * graphWidth) / (dataPoints.size() - 1);
}

void GraphRenderer::clear() { M5.Display.fillScreen(TFT_BLACK); }

void GraphRenderer::draw() {
    // 背景をクリア
    M5.Display.fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, TFT_BLACK);

    // タイトルを描画（1280x720用に調整）
    String title = GRAPH_TITLE;
    if (config) {
        title = config->getGraphConfig().title;
    }

    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setFont(&fonts::lgfxJapanMinchoP_24);
    M5.Display.drawString(title, 20, 20);

    drawAxes();
    drawGrid();
    drawLabels();

    if (!dataPoints.empty()) {
        drawDataLine();
    }
}

void GraphRenderer::drawAxes() {
    // X軸
    M5.Display.drawLine(graphX, graphY + graphHeight, graphX + graphWidth, graphY + graphHeight,
                        TFT_WHITE);

    // Y軸
    M5.Display.drawLine(graphX, graphY, graphX, graphY + graphHeight, TFT_WHITE);
}

void GraphRenderer::drawGrid() {
    M5.Display.setTextColor(TFT_DARKGREY);

    // 横のグリッド線（Y軸方向）
    for (int i = 1; i < 5; i++) {
        int y = graphY + (i * graphHeight) / 5;
        M5.Display.drawLine(graphX, y, graphX + graphWidth, y, TFT_DARKGREY);
    }

    // 縦のグリッド線（X軸方向）
    for (int i = 1; i < 8; i++) {
        int x = graphX + (i * graphWidth) / 8;
        M5.Display.drawLine(x, graphY, x, graphY + graphHeight, TFT_DARKGREY);
    }
}

void GraphRenderer::drawLabels() {
    M5.Display.setTextColor(TFT_WHITE);

    String yLabel = Y_AXIS_LABEL;
    String xLabel = X_AXIS_LABEL;

    if (config) {
        const auto &graphConfig = config->getGraphConfig();
        yLabel = graphConfig.yAxisLabel;
        xLabel = graphConfig.xAxisLabel;
    }

    // Y軸ラベル（縦書き風に配置）
    M5.Display.setFont(&fonts::lgfxJapanMinchoP_12);
    M5.Display.drawString(yLabel, 10, graphY + graphHeight / 2);

    // Y軸の値
    for (int i = 0; i <= 5; i++) {
        int y = graphY + graphHeight - (i * graphHeight) / 5;
        float value = minValue + (i * (maxValue - minValue)) / 5;
        M5.Display.setFont(&fonts::lgfxJapanMinchoP_12);
        M5.Display.drawString(String(value, 1), graphX - 80, y - 10);
    }

    // X軸ラベル
    M5.Display.drawString(xLabel, graphX + graphWidth / 2 - 30, graphY + graphHeight + 40);

    // 時間ラベル（簡略化）
    if (!dataPoints.empty()) {
        // 開始時刻
        String startLabel = "24h ago";
        if (config) {
            startLabel = String(config->getSystemConfig().dataHours) + "h ago";
        }
        M5.Display.drawString(startLabel, graphX - 30, graphY + graphHeight + 60);

        // 終了時刻
        M5.Display.drawString("Now", graphX + graphWidth - 30, graphY + graphHeight + 60);
    }
}

void GraphRenderer::drawDataLine() {
    if (dataPoints.size() < 2)
        return;

    uint32_t lineColor = TFT_YELLOW;
    if (config) {
        lineColor = config->getDataSourceConfig().color;
    }

    for (size_t i = 1; i < dataPoints.size(); i++) {
        int x1 = mapTimeToX(i - 1);
        int y1 = mapValueToY(dataPoints[i - 1].value);
        int x2 = mapTimeToX(i);
        int y2 = mapValueToY(dataPoints[i].value);

        M5.Display.drawLine(x1, y1, x2, y2, lineColor);

        // データポイントをドットで表示
        M5.Display.fillCircle(x2, y2, 2, TFT_YELLOW);
    }

    // 最初のポイントもドットで表示
    if (!dataPoints.empty()) {
        int x1 = mapTimeToX(0);
        int y1 = mapValueToY(dataPoints[0].value);
        M5.Display.fillCircle(x1, y1, 1, TFT_YELLOW);
    }
}

void GraphRenderer::drawLatestValue(float value) {
    // 最新値を画面右側に大きく表示（1280x720用に調整）
    String unit = "units";

    if (config) {
        const auto &dataConfig = config->getDataSourceConfig();
        unit = dataConfig.unit;
    }

    M5.Display.setTextColor(TFT_WHITE);

    M5.Display.setFont(&fonts::lgfxJapanMinchoP_36);
    M5.Display.drawString(String(int(value)), graphX + graphWidth + 50, 160);
    M5.Display.drawString("[w]", graphX + graphWidth + 50, 220);

}