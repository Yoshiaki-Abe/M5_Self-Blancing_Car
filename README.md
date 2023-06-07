# M5_Self-Blancing_Car
M5Stack-greyを使った車輪型の倒立振子ロボット

## 概要
- M5Stack-Greyを使って倒立振子ロボットを作った
- BlynkというiPhoneアプリを使ってラジコンみたいに操作できる
- 300gぐらいならおもりを載せても動作可
- 楽しい

## 背景
- M5Stack Greyを手に入れたので，何か作ろう
- M5Stack Greyの特徴が活かせるものがいい？
    - WiFi/Bluetoothによる無線接続
    - 最低限のGPIOポート
    - マイコン用Li-Poバッテリ内蔵
    - IMU内蔵，傾きの検知ができる
- 倒立振子は制御工学でよく扱う題材，一度作ってみたいので，これを機にやってみる

## ハードウェア
- 3DプリンタでABS樹脂を使って外装を作成
- 車輪は3Dプリンタ製のホイールにOリングをはめて作成
- 転倒時の衝撃対策にウレタン緩衝材を搭載
- ステッピングモータ
    - 入手性が良く，制御しやすい
    - トルクがそれなりにあるので，ダイレクトドライブで使える．ギヤのバックラッシは倒立振子の挙動を不安定にさせやすい．

  <img src="images/robot.png" height=320px>

<br>

- 内部  

  <img src="images/coverless.png" height=320px>

## システム構成

  <img src="images/system_overview.png" height=320px>

- Blynkアプリの操作画面  

  <img src="images/control_interface.png" width=320px>

## 制御

  <img src="images/control_diagram.png" width=320px>