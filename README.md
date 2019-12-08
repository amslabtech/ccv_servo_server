# ccv_servo_server
## サーボシステムへのMQTTゲートウェイ
MQTT Service for servo system: publisher and subscriber



## 機能
- MQTTサブスクライバを起動し，servo_writeトピックに受けたデータを，サーボシステムに対して出力する．
- MQTTパプリッシャからサーボシステムのカレントデータを servo_read　トピックへ発信する．


## Packet
see ccv_servo_structure.hpp

