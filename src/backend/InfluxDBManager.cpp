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

std::vector<DataPoint> InfluxDBManager::getData() {
    std::vector<DataPoint> dataPoints;
    
    if (!client || !isConnected()) {
        Serial.println("InfluxDB not connected");
        return dataPoints;
    }
    
    String query = buildFluxQuery();
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

    String query;
    query  = "import \"date\"\n";
    query += "start = date.truncate(t: now(), unit: 1mo)\n";
    query += "from(bucket: \"";
    query += INFLUXDB_BUCKET;
    query += "\")";
    query += " |> range(start: start)";
    query += " |> filter(fn: (r) => r[\"_measurement\"] == \"";
    query += measurement;
    query += "\")";
    query += " |> filter(fn: (r) => r[\"_field\"] == \"";
    query += FIELD_NAME_CUMULATIVE_ENERGY_KWH;
    query += "\")";
    query += " |> sort(columns: [\"_time\"], desc: false)";

    Serial.println("Executing monthly energy query: " + query);

    FluxQueryResult result = client->query(query);

    if (result.getError() != "") {
        Serial.printf("Monthly query error: %s\n", result.getError().c_str());
        Serial.printf("InfluxDB error: %s\n", client->getLastErrorMessage().c_str());
        result.close();
        return false;
    }

    bool hasData = false;
    float firstValue = 0.0f;
    float lastValue = 0.0f;

    while (result.next()) {
        FluxValue valueFlux = result.getValueByName("_value");
        if (valueFlux.isNull()) {
            continue;
        }

        float value = valueFlux.getDouble();
        if (!hasData) {
            firstValue = value;
            hasData = true;
        }
        lastValue = value;
    }

    result.close();

    if (!hasData) {
        Serial.println("No cumulative energy data found for current month");
        return false;
    }

    monthlyUsage = lastValue - firstValue;
    if (monthlyUsage < 0) {
        monthlyUsage = 0;
    }

    Serial.println("Monthly energy usage: " + String(monthlyUsage));
    return true;
}

bool InfluxDBManager::isConnected() {
    // Wi-Fi接続状態とInfluxDBクライアントの状態をチェック
    return WiFi.status() == WL_CONNECTED && client != nullptr;
}