#ifndef __GAME_PROTOCOL_H
#define __GAME_PROTOCOL_H

#include "main.h"
#include "buzzer.h"

/* 发布当前开发板上线 STATUS 消息。 */
void GameProtocol_PublishStatus(void);

/* 保存 ESP8266 获取到的本机 IP，后续 STATUS 会自动携带该 IP。 */
void GameProtocol_SetDeviceIp(const char *ip);

/* 发布按键移动 INPUT 消息，move 取值范围为 -1、0、1。 */
void GameProtocol_PublishInput(int8_t move);

/* 解析 UE 下发的 RESULT payload，返回需要播放的蜂鸣器事件。 */
BuzzerEvent_t GameProtocol_ParseResult(const char *payload);

/* 判断 RESULT payload 是否表示游戏退出或设备离线。 */
uint8_t GameProtocol_IsExitResult(const char *payload);

/* 解析 MQTT 原始 PUBLISH 包，如果是本玩家 RESULT 主题则返回蜂鸣器事件。 */
BuzzerEvent_t GameProtocol_ParseMqttPacket(uint8_t *data, uint16_t len);

/* 判断 MQTT 原始 PUBLISH 包是否表示游戏退出或设备离线。 */
uint8_t GameProtocol_IsExitMqttPacket(uint8_t *data, uint16_t len);

#endif
