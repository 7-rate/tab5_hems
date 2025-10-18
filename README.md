# TODOリスト

## 概要
InfluxDBから取得したデータ(24h分)をグラフにして表示するアプリケーションをM5Stack Tab5上で開発します。

## 環境
- デバイス: M5Stack Tab5

## 機能要件
1. InfluxDBから3分毎にデータを取得し、24h分のデータをグラフ表示する。
2. 最新のデータを数字で表示する。
3. グラフは折れ線とする。
4. デバイスの表示は横レイアウトとする。
5. backendとfrontendはそれぞれタスクを分離する。

## 非機能要件
1. InfluxDBやWi-Fiの接続情報は`main.cpp`と分離すること(`env.h`などで定義する)。
2. 簡単に様々なデータ、横軸、縦軸を変更できるような設計とすること。
3. `src`以下に`frontend`、`backend`ディレクトリを作成し、InfluxDBやWi-Fiなどのロジックは`backend`に実装し、UIは`frontend`に実装する。

## TODOリスト

1. **プロジェクト構造の整理**
   - [ ] `src/frontend`と`src/backend`ディレクトリを作成する。
   - [ ] `include/env.h`を作成し、Wi-FiとInfluxDBの接続情報を定義する。

2. **Wi-Fi接続機能の実装**
   - [ ] `backend`にWi-Fi接続処理を実装する。
   - [ ] `env.h`で定義した情報を使用する。

3. **InfluxDBクライアントの実装**
   - [ ] `backend`にInfluxDBからデータを取得するクライアントを実装する。
   - [ ] HTTPリクエストの送信、レスポンスの解析を行う。

4. **グラフ描画機能の実装**
   - [ ] `frontend`にM5GFXライブラリを使用して折れ線グラフを描画する機能を実装する。
   - [ ] 軸やラベルの描画を行う。

5. **メインループの実装**
   - [ ] 3分ごとにデータを取得し、グラフと最新の数値を更新するメインループを実装する。

6. **横レイアウトへの対応**
   - [ ] デバイスの画面を横向きに設定する。
   - [ ] 各UI要素を適切に配置する。

7. **設定の柔軟性を高めるリファクタリング**
   - [ ] 取得するデータ、グラフの軸などを簡単に変更できるよう、設定を外部化・構造化する。

## トラブルシューティング

### 🚨 実行時エラー対応 (2025/10/14発生)

**エラー内容:**
```
E (2629) sdmmc_common: sdmmc_init_ocr: send_op_cond (1) returned 0x107
E (2629) sdio_wrapper: sdmmc_card_init failed
E (6519) H_SDIO_DRV: sdio card init failed
FreeRTOS: FreeRTOS Task "sdio_read" should not return, Aborting now!
```

**考えられる原因:**
8. **Wi-Fi初期化エラーの修正**
   - [ ] ESP-HOSTED WiFiドライバーの初期化失敗 (SDIOカード初期化エラー)
   - [ ] M5Stack Tab5のWi-Fi設定が正しくない可能性
   - [ ] ハードウェア固有のWi-Fi初期化コードが必要

9. **M5Stack Tab5固有の設定問題**
   - [ ] platformio.iniのboard設定が`esp32-p4-evboard`で正しいか確認
   - [ ] M5Stack Tab5専用のboard定義が必要な可能性
   - [ ] Wi-Fiライブラリの互換性問題

10. **デバッグと代替案の検討**
    - [ ] Wi-Fi機能を一時的に無効化してディスプレイテストを実行
    - [ ] M5Stack Tab5公式サンプルコードでWi-Fi初期化方法を確認
    - [ ] ESP32-P4とESP-HOSTEDの組み合わせに関する制約を調査
    - [ ] ログレベルを下げて詳細なエラー情報を取得

**緊急対応策:**
- Wi-Fi接続を無効化して基本的なディスプレイ機能をテスト
- M5Stack Tab5の公式Wi-Fi例から正しい初期化コードを参照
- board設定を見直してTab5専用の設定を適用