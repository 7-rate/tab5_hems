#include <M5Unified.h>
#include "backend/WifiManager.h"
#include "backend/InfluxDBManager.h"
#include "frontend/GraphRenderer.h"
#include "../include/env.h"
#include "../include/ConfigManager.h"

// グローバル変数
WifiManager wifiManager;
InfluxDBManager influxManager;
GraphRenderer graphRenderer;
ConfigManager configManager;

unsigned long lastDataUpdate = 0;
unsigned long dataUpdateInterval;

void updateData() {
    Serial.println("Updating data from InfluxDB...");

    // データ取得
    std::vector<DataPoint> data = influxManager.getData();

    if (!data.empty()) {
        // グラフ描画
        graphRenderer.setData(data);
        graphRenderer.draw();

        // 最新値表示
        float latestValue = data.back().value;
        graphRenderer.drawLatestValue(latestValue);

        Serial.println("Data updated successfully. Points: " + String(data.size()));
        Serial.println("Latest value: " + String(latestValue));
    } else {
        Serial.println("No data received from InfluxDB");

        // エラーメッセージを表示
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.drawString("No Data Available", 100, 100);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.drawString("Check InfluxDB connection", 100, 130);
    }

    lastDataUpdate = millis();
}

void setup() {
    Serial.begin(115200);

    // M5Stackの初期化
    auto cfg = M5.config();
    M5.begin(cfg);

    // 横レイアウトに設定
    M5.Display.setRotation(1);

    Serial.println("M5Stack Tab5 HEMS Monitor Starting...");

    // 設定の初期化とプリセット読み込み
    // 必要に応じて以下のプリセットを選択してください：
    // configManager.loadTemperatureConfig();
    // configManager.loadHumidityConfig();
    // configManager.loadPowerConfig();
    // または、カスタム設定：
    // configManager.loadCustomConfig("your_measurement", "your_field");

    // データ更新間隔を設定から取得
    dataUpdateInterval = configManager.getSystemConfig().updateIntervalMinutes * 60 * 1000;

    // 各コンポーネントに設定を適用
    influxManager.setConfig(&configManager);
    graphRenderer.setConfig(&configManager);

    // Wi-Fi接続
    if (wifiManager.connect()) {
        Serial.println("Wi-Fi connected successfully");
    } else {
        Serial.println("Wi-Fi connection failed");
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.drawString("Wi-Fi Connection Failed", 100, 100);
        return;
    }

    // InfluxDB接続
    if (influxManager.connect()) {
        Serial.println("InfluxDB connected successfully");
    } else {
        Serial.println("InfluxDB connection failed");
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.drawString("InfluxDB Connection Failed", 100, 130);
        return;
    }

    // 初期画面表示
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Loading data...", 100, 50);

    // 初回データ取得
    updateData();
}

void loop() {
    M5.update();

    // 設定で指定された間隔でデータを更新
    if (millis() - lastDataUpdate >= dataUpdateInterval) {
        updateData();
    }

    // Wi-Fi接続状態の監視
    if (!wifiManager.isWifiConnected()) {
        Serial.println("Wi-Fi disconnected, attempting to reconnect...");
        if (wifiManager.connect()) {
            Serial.println("Wi-Fi reconnected");
        }
    }

    delay(1000); // 1秒間隔でチェック
}
