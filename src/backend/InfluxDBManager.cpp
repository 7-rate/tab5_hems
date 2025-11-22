#include "InfluxDBManager.h"
#include "../../include/env.h"
#include "../../include/ConfigManager.h"
#include <WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

InfluxDBManager::InfluxDBManager() : config(nullptr), client(nullptr) {
}

InfluxDBManager::~InfluxDBManager() {
    if (client) {
        delete client;
        client = nullptr;
    }
}

void InfluxDBManager::setConfig(ConfigManager* configManager) {
    config = configManager;
}

bool InfluxDBManager::connect() {
    if (client) {
        delete client;
    }
    
    // InfluxDBクライアントを初期化
    client = new ::InfluxDBClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

    
    // サーバー証明書の検証を無効化（ローカル環境の場合）
    client->setInsecure();
    
    // 接続テスト
    if (client->validateConnection()) {
        Serial.println("Connected to InfluxDB: " + client->getServerUrl());
        return true;
    } else {
        Serial.print("InfluxDB connection failed: ");
        Serial.println(client->getLastErrorMessage());
        return false;
    }
}

String InfluxDBManager::buildFluxQuery() {
    String measurement = MEASUREMENT_NAME;
    String field = FIELD_NAME_INSTANT_POWER_W;
    int hours = DATA_HOURS;
    int intervalMinutes = DATA_INTERVAL_MINUTES;
    
    // ConfigManagerが設定されている場合は、その設定を使用
    if (config) {
        const auto& dataConfig = config->getDataSourceConfig();
        const auto& systemConfig = config->getSystemConfig();
        
        measurement = dataConfig.measurement;
        field = dataConfig.field;
        hours = systemConfig.dataHours;
        intervalMinutes = systemConfig.updateIntervalMinutes;
    }
    
    String query = "from(bucket: \"";
    query += INFLUXDB_BUCKET;
    query += "\")";
    query += " |> range(start: -";
    query += String(hours);
    query += "h)";
    query += " |> filter(fn: (r) => r[\"_measurement\"] == \"";
    query += measurement;
    query += "\")";
    query += " |> filter(fn: (r) => r[\"_field\"] == \"";
    query += field;
    query += "\")";
    query += " |> aggregateWindow(every: ";
    query += String(intervalMinutes);
    query += "m, fn: mean, createEmpty: false)";
    query += " |> yield(name: \"mean\")";
    
    return query;
}

std::vector<DataPoint> InfluxDBManager::getData(int hours) {
    std::vector<DataPoint> dataPoints;
    
    if (!client || !isConnected()) {
        Serial.println("InfluxDB not connected");
        return dataPoints;
    }
    
    // hoursパラメータが指定されていない場合はデフォルト値を使用
    if (hours <= 0) {
        hours = DATA_HOURS;
        if (config) {
            hours = config->getSystemConfig().dataHours;
        }
    }
    
    String measurement = MEASUREMENT_NAME;
    String field = FIELD_NAME_INSTANT_POWER_W;
    int intervalMinutes = DATA_INTERVAL_MINUTES;
    
    if (config) {
        const auto& dataConfig = config->getDataSourceConfig();
        measurement = dataConfig.measurement;
        field = dataConfig.field;
        intervalMinutes = config->getSystemConfig().updateIntervalMinutes;
    }
    
    String query = "from(bucket: \"";
    query += INFLUXDB_BUCKET;
    query += "\")";
    query += " |> range(start: -";
    query += String(hours);
    query += "h)";
    query += " |> filter(fn: (r) => r[\"_measurement\"] == \"";
    query += measurement;
    query += "\")";
    query += " |> filter(fn: (r) => r[\"_field\"] == \"";
    query += field;
    query += "\")";
    query += " |> aggregateWindow(every: ";
    query += String(intervalMinutes);
    query += "m, fn: mean, createEmpty: false)";
    query += " |> yield(name: \"mean\")";
    
    Serial.println("Executing Flux query: " + query);
    
    // Fluxクエリを実行
    FluxQueryResult result = client->query(query);

    // エラーチェック
    if(result.getError() != "") {
        Serial.printf("Query error: %s\n", result.getError().c_str());
        Serial.printf("InfluxDB error: %s\n", client->getLastErrorMessage().c_str());
        return dataPoints;
    }

    // 結果を解析
    while (result.next()) {
        DataPoint point;
        
        // タイムスタンプを取得
        FluxValue timeValue = result.getValueByName("_time");
        if (!timeValue.isNull()) {
            FluxDateTime time = timeValue.getDateTime();
            point.timestamp = time.format("%Y-%m-%d %H:%M:%S");
            Serial.println("Timestamp: " + point.timestamp);
        }
        
        // 値を取得
        FluxValue valueFlux = result.getValueByName("_value");
        if (!valueFlux.isNull()) {
            point.value = valueFlux.getDouble();
            Serial.println("Value: " + String(point.value));
        }
        
        if (!point.timestamp.isEmpty()) {
            dataPoints.push_back(point);
        }
    }
    
    // エラーチェック
    if (result.getError() != "") {
        Serial.print("Query result error: ");
        Serial.println(result.getError());
    }
    
    result.close();
    
    Serial.println("InfluxDB data retrieved successfully");
    Serial.println("Data points count: " + String(dataPoints.size()));
    
    return dataPoints;
}

float InfluxDBManager::getLatestValue() {
    std::vector<DataPoint> data = getData();
    if (data.size() > 0) {
        return data.back().value; // 最後のデータポイントを返す
    }
    return 0.0;
}

bool InfluxDBManager::getMonthlyEnergyUsage(float &monthlyUsage) {
    monthlyUsage = 0.0f;

    if (!client || !isConnected()) {
        Serial.println("InfluxDB not connected");
        return false;
    }

    String measurement = MEASUREMENT_NAME;
    if (config) {
        measurement = config->getDataSourceConfig().measurement;
    }

    // ステップ1: 最新の1件を取得
    String latestQuery = "from(bucket: \"";
    latestQuery += INFLUXDB_BUCKET;
    latestQuery += "\")";
    latestQuery += " |> range(start: -30d)";
    latestQuery += " |> filter(fn: (r) => r[\"_measurement\"] == \"";
    latestQuery += measurement;
    latestQuery += "\")";
    latestQuery += " |> filter(fn: (r) => r[\"_field\"] == \"";
    latestQuery += FIELD_NAME_CUMULATIVE_ENERGY_KWH;
    latestQuery += "\")";
    latestQuery += " |> last()";

    Serial.println("Executing latest value query: " + latestQuery);

    FluxQueryResult latestResult = client->query(latestQuery);

    if (latestResult.getError() != "") {
        Serial.printf("Latest query error: %s\n", latestResult.getError().c_str());
        latestResult.close();
        return false;
    }

    float latestValue = 0.0f;
    String latestTime = "";
    bool hasLatest = false;

    if (latestResult.next()) {
        FluxValue valueFlux = latestResult.getValueByName("_value");
        FluxValue timeFlux = latestResult.getValueByName("_time");
        
        if (!valueFlux.isNull() && !timeFlux.isNull()) {
            latestValue = valueFlux.getDouble();
            FluxDateTime time = timeFlux.getDateTime();
            latestTime = time.format("%Y-%m-%dT%H:%M:%SZ");
            hasLatest = true;
            Serial.println("Latest value: " + String(latestValue) + " at " + latestTime);
        }
    }

    latestResult.close();

    if (!hasLatest) {
        Serial.println("No latest cumulative energy data found");
        return false;
    }

    // ステップ2: 月初の日付を計算 (latestTimeから)
    // latestTimeの形式: "2025-10-26T12:34:56Z"
    String yearStr = latestTime.substring(0, 4);
    String monthStr = latestTime.substring(5, 7);
    String monthStartTime = yearStr + "-" + monthStr + "-01T00:00:00Z";

    Serial.println("Month start time: " + monthStartTime);

    // ステップ3: 月初のcumulative_energy_kwhを取得（月初のデータが無い場合は今月の最古データを取得）
    // 今月の最後の日を計算
    int month = monthStr.toInt();
    int year = yearStr.toInt();
    int nextMonth = month + 1;
    int nextYear = year;
    if (nextMonth > 12) {
        nextMonth = 1;
        nextYear++;
    }
    String monthEndTime = String(nextYear) + "-" + (nextMonth < 10 ? "0" : "") + String(nextMonth) + "-01T00:00:00Z";

    String monthStartQuery = "from(bucket: \"";
    monthStartQuery += INFLUXDB_BUCKET;
    monthStartQuery += "\")";
    monthStartQuery += " |> range(start: " + monthStartTime + ", stop: " + monthEndTime + ")";
    monthStartQuery += " |> filter(fn: (r) => r[\"_measurement\"] == \"";
    monthStartQuery += measurement;
    monthStartQuery += "\")";
    monthStartQuery += " |> filter(fn: (r) => r[\"_field\"] == \"";
    monthStartQuery += FIELD_NAME_CUMULATIVE_ENERGY_KWH;
    monthStartQuery += "\")";
    monthStartQuery += " |> first()";

    Serial.println("Executing month start query: " + monthStartQuery);

    FluxQueryResult monthStartResult = client->query(monthStartQuery);

    if (monthStartResult.getError() != "") {
        Serial.printf("Month start query error: %s\n", monthStartResult.getError().c_str());
        monthStartResult.close();
        return false;
    }

    float monthStartValue = 0.0f;
    bool hasMonthStart = false;

    if (monthStartResult.next()) {
        FluxValue valueFlux = monthStartResult.getValueByName("_value");
        
        if (!valueFlux.isNull()) {
            monthStartValue = valueFlux.getDouble();
            hasMonthStart = true;
            Serial.println("Month start value: " + String(monthStartValue));
        }
    }

    monthStartResult.close();

    if (!hasMonthStart) {
        Serial.println("No data found for current month");
        return false;
    }

    // ステップ4: 差分を計算
    monthlyUsage = latestValue - monthStartValue;
    if (monthlyUsage < 0) {
        monthlyUsage = 0;
    }

    Serial.println("Monthly energy usage: " + String(monthlyUsage) + " kWh");
    return true;
}

bool InfluxDBManager::isConnected() {
    // Wi-Fi接続状態とInfluxDBクライアントの状態をチェック
    return WiFi.status() == WL_CONNECTED && client != nullptr;
}