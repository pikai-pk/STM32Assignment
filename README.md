# FinalAssignment

嵌入式期末作业：基于 MQTT 的 UE5 双人弹球游戏与 STM32 控制器联动。

## 项目简介

本项目将 STM32 嵌入式控制器、MQTT 通信和 Unreal Engine 5 游戏端结合，实现通过实体按键/控制器操作 UE5 双人 Pong 游戏的交互效果。

## 视频展示

[嵌入式期末作业视频展示](https://www.bilibili.com/video/BV18ULD6iEjP?vd_source=e03c2edd09090734ede0815cdc202303)

## 目录结构

- `01_UE_Project/`：UE5 工程源码
- `02_UE_Package/`：Windows 打包版本
- `03_STM32_Project/`：STM32 控制端工程

## STM32 端配置修改

STM32 控制端的 Wi-Fi、MQTT 和玩家编号配置集中在：

`03_STM32_Project/PongCtrl/myDrivers/app_config.h`

一般只需要修改这个文件，不需要改 `bsp_esp8266.c` 或 `mqtt_client.c`。

### 连接自己的热点

如果要让 STM32 连接自己的手机热点、电脑热点或路由器 Wi-Fi，修改下面两行：

```c
#define WIFI_SSID                 "你的热点名称"
#define WIFI_PASSWORD             "你的热点密码"
```

例如手机热点名称是 `MyPhone`，密码是 `12345678`：

```c
#define WIFI_SSID                 "MyPhone"
#define WIFI_PASSWORD             "12345678"
```

注意事项：

- ESP8266 通常只支持 2.4GHz Wi-Fi，手机热点建议开启 2.4GHz 或“兼容模式”。
- 热点名称和密码要区分大小写。
- 如果密码为空，需要确认 ESP8266 AT 固件是否支持空密码热点。

### 修改 MQTT 服务器地址

MQTT Broker 地址同样在 `app_config.h` 中修改：

```c
#define MQTT_BROKER_HOST          "192.168.137.1"
#define MQTT_BROKER_PORT          1883
```

`MQTT_BROKER_HOST` 要填写 MQTT 服务器在局域网里的真实 IP，不能写 `127.0.0.1`。`127.0.0.1` 只代表 STM32/ESP8266 自己，不代表电脑。

如果 MQTT Broker 运行在电脑上，操作步骤是：

1. 让电脑和 STM32 连接到同一个热点或同一个路由器。
2. 在电脑上打开命令行，执行 `ipconfig`。
3. 找到当前网络的 IPv4 地址，例如 `192.168.137.1` 或 `192.168.1.23`。
4. 把这个地址填入 `MQTT_BROKER_HOST`。

示例：

```c
#define MQTT_BROKER_HOST          "192.168.1.23"
#define MQTT_BROKER_PORT          1883
```

如果使用云服务器或局域网 MQTT 服务器，也可以填写服务器 IP 或域名：

```c
#define MQTT_BROKER_HOST          "broker.emqx.io"
#define MQTT_BROKER_PORT          1883
```

如果 MQTT Broker 设置了账号密码，修改：

```c
#define MQTT_USERNAME             "你的MQTT用户名"
#define MQTT_PASSWORD             "你的MQTT密码"
```

如果没有账号密码，保持为空字符串：

```c
#define MQTT_USERNAME             ""
#define MQTT_PASSWORD             ""
```

### 修改玩家编号

两块 STM32 需要使用不同的玩家编号。烧录第一块板时设置：

```c
#define PLAYER_ID                 1
```

烧录第二块板时设置：

```c
#define PLAYER_ID                 2
```

代码会根据 `PLAYER_ID` 自动选择不同的 MQTT Client ID 和主题：

- 玩家 1：`GAME/PLAYER/1/STATUS`、`GAME/PLAYER/1/INPUT`、`GAME/PLAYER/1/RESULT`
- 玩家 2：`GAME/PLAYER/2/STATUS`、`GAME/PLAYER/2/INPUT`、`GAME/PLAYER/2/RESULT`

两块板子的 `MQTT_CLIENT_ID` 必须不同，否则 MQTT Broker 会把前一块板挤下线。本项目已经通过 `PLAYER_ID` 自动区分：

```c
#if (PLAYER_ID == 1)
#define MQTT_CLIENT_ID            "STM32_PLAYER_1"
#else
#define MQTT_CLIENT_ID            "STM32_PLAYER_2"
#endif
```

### 修改后重新烧录

修改 `app_config.h` 后，需要重新编译并烧录 STM32 工程：

1. 打开 `03_STM32_Project/PongCtrl/MDK-ARM/PongCtrl.uvprojx`。
2. 修改 `myDrivers/app_config.h`。
3. 编译工程。
4. 将程序烧录到 STM32。
5. 打开串口调试工具查看日志，确认出现 Wi-Fi connected、MQTT CONNECT OK 等信息。

## 说明

仓库已启用 Git LFS，用于管理 UE 资源、打包文件和其他大文件。克隆仓库后如需完整拉取大文件，请先安装 Git LFS，然后执行：

```bash
git lfs pull
```
