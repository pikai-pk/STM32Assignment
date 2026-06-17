#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/*
 * 项目运行参数集中配置
 *
 * 你后续通常只需要改这个文件：
 * 1. 修改 Wi-Fi 热点名称和密码
 * 2. 修改电脑上 MQTT Broker 的局域网 IP
 * 3. 给两块 STM32 分别设置不同的 PLAYER_ID 和 MQTT_CLIENT_ID
 */

/* ===================== 玩家身份配置 ===================== */
/* Player1 控制下方挡板，Player2 控制上方挡板。
 * 烧录第一块板时填 1，烧录第二块板时填 2。
 */
#define PLAYER_ID                 2

/* 每块板子的 MQTT Client ID 必须不同，否则两块板会互相顶掉连接。 */
#if (PLAYER_ID == 1)
#define MQTT_CLIENT_ID            "STM32_PLAYER_1"
#else
#define MQTT_CLIENT_ID            "STM32_PLAYER_2"
#endif

/* ===================== Wi-Fi 热点配置 ===================== */
/* STM32 通过 ESP8266 连接的热点名称。 */
#define WIFI_SSID                 "chillchill"

/* STM32 通过 ESP8266 连接的热点密码。 */
#define WIFI_PASSWORD             "chilichill"

/* ===================== MQTT Broker 配置 ===================== */
/* 电脑或服务器在局域网中的真实 IP，不能写 127.0.0.1。
 * 示例：电脑连接同一个热点后，ipconfig 查到 192.168.1.10，就填这个地址。
 */
#define MQTT_BROKER_HOST          "192.168.137.1"

/* Mosquitto/EMQX 默认 MQTT 端口一般是 1883。 */
#define MQTT_BROKER_PORT          1883

/* 如果你的 Broker 没有账号密码，就保持为空字符串。 */
#define MQTT_USERNAME             ""
#define MQTT_PASSWORD             ""

/* MQTT 心跳间隔，单位：秒。 */
#define MQTT_KEEPALIVE_SECONDS    60

/* ===================== Pong 项目主题配置 ===================== */
#if (PLAYER_ID == 1)
#define TOPIC_STATUS              "GAME/PLAYER/1/STATUS"
#define TOPIC_INPUT               "GAME/PLAYER/1/INPUT"
#define TOPIC_RESULT              "GAME/PLAYER/1/RESULT"
#else
#define TOPIC_STATUS              "GAME/PLAYER/2/STATUS"
#define TOPIC_INPUT               "GAME/PLAYER/2/INPUT"
#define TOPIC_RESULT              "GAME/PLAYER/2/RESULT"
#endif

/* UE5 可额外发布或调试观察的全局游戏状态主题。 */
#define TOPIC_GAME_STATE          "GAME/STATE"

/* ===================== 兼容旧驱动宏名 ===================== */
/* 下面这些宏用于兼容参考驱动里的旧命名，业务代码建议优先使用上面的新名字。 */
#define WIFI_PASS                 WIFI_PASSWORD
#define THINGS_CLOUD_HOST         MQTT_BROKER_HOST
#define THINGS_CLOUD_PORT         MQTT_BROKER_PORT
#define DEVICE_NAME               MQTT_CLIENT_ID
#define ACCESS_TOKEN              MQTT_USERNAME
#define PROJECT_KEY               MQTT_PASSWORD
#define MQTT_PUB_TOPIC            TOPIC_STATUS
#define MQTT_SUB_TOPIC            TOPIC_RESULT
#define MQTT_SUB_TOPIC_COMMAND    TOPIC_RESULT

#endif
