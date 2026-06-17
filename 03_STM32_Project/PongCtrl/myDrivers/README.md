# myDrivers 说明

这个目录存放 PongCtrl 项目需要的自定义驱动和协议文件。

## 配置入口

请优先修改 `app_config.h`：

- `PLAYER_ID`：当前板子是 1 号玩家还是 2 号玩家
- `WIFI_SSID`：ESP8266 要连接的热点名称
- `WIFI_PASSWORD`：热点密码
- `MQTT_BROKER_HOST`：电脑或服务器的局域网 IP，不能写 `127.0.0.1`
- `MQTT_BROKER_PORT`：MQTT 端口，通常是 `1883`
- `MQTT_USERNAME` / `MQTT_PASSWORD`：Broker 账号密码，没有就保持空字符串

## 文件用途

- `bsp_esp8266.c/.h`：ESP8266 AT 指令、Wi-Fi 连接、TCP 透传
- `bsp_uart_fifo.c/.h`：USART6 接收环形缓冲区
- `mqtt_client.c/.h`：MQTT CONNECT、PUBLISH、SUBSCRIBE、PING
- `mqtt_config.h`：兼容旧代码的配置入口，实际包含 `app_config.h`
- `keys.c/.h`：KEY1/KEY2 按键扫描
- `led.c/.h`：PE8-PE15 LED 连接状态显示

## MQTT 主题

STM32 发布：

- `GAME/PLAYER/1/STATUS`
- `GAME/PLAYER/2/STATUS`
- `GAME/PLAYER/1/INPUT`
- `GAME/PLAYER/2/INPUT`

STM32 订阅：

- `GAME/PLAYER/1/RESULT`
- `GAME/PLAYER/2/RESULT`
