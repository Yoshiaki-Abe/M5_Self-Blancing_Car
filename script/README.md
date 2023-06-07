# M5Stackに書き込むソースコード
Arduino IDEを使って書き込める  

## ソースコードの特徴
- Bluetooth通信部と倒立・運動制御部は処理させるコアを分けて，マルチタスクさせている
  - 制御処理へ無線通信の遅延の影響を与えずに，安定して2msの制御周期を実現させるため
  - M5Stack内部のマイコン，ESP32はデュアルコアで，無線通信用のコアと汎用処理用のコアに分かれている
- ステッピングモータの制御はタイマ割り込みを使用

## 内容物(self_bala_robot/)
- self_bala_robot.ino
  - メインプログラム

- PIDController.h/cpp
  - PID制御器のクラス

- Stepper.h/cpp
  - ステッピングモータ制御用クラス

- Kalman.h/cpp
  - 使用させていただいたKalman Filterのプログラム
  - Kristian Lauszus, TKJ Electronics
  - Web      :  http://www.tkjelectronics.com
  - e-mail   :  kristianl@tkjelectronics.com