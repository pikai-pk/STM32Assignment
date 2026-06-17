#include "game_protocol.h"
#include "mqtt_client.h"
#include "mqtt_config.h"
#include <stdio.h>
#include <string.h>

static char g_device_ip[32] = "unknown";

/* 保存 ESP8266 的本机 IP，供 STATUS 消息上报给 UE。 */
void GameProtocol_SetDeviceIp(const char *ip)
{
    if ((ip == NULL) || (ip[0] == '\0'))
    {
        snprintf(g_device_ip, sizeof(g_device_ip), "%s", "unknown");
        return;
    }

    snprintf(g_device_ip, sizeof(g_device_ip), "%s", ip);
}

/* 发布当前 STM32 的上线状态，UE 用它记录玩家编号和开发板 IP。 */
void GameProtocol_PublishStatus(void)
{
    char payload[128];

    snprintf(payload,
             sizeof(payload),
             "{\"player\":%d,\"ip\":\"%s\",\"status\":\"online\"}",
             PLAYER_ID,
             g_device_ip);

    MQTT_Publish(TOPIC_STATUS, payload);
}

/* 发布按键方向，move=-1、0、1 分别表示两个方向和停止。 */
void GameProtocol_PublishInput(int8_t move)
{
    char payload[64];

    if (move < -1)
    {
        move = -1;
    }
    else if (move > 1)
    {
        move = 1;
    }

    snprintf(payload, sizeof(payload), "{\"player\":%d,\"move\":%d}", PLAYER_ID, move);
    printf("[MQTT] Publish INPUT %s -> %s\r\n", TOPIC_INPUT, payload);
    MQTT_Publish(TOPIC_INPUT, payload);
}

/* 判断 RESULT payload 是否表示游戏退出或设备离线。 */
uint8_t GameProtocol_IsExitResult(const char *payload)
{
    if (payload == NULL)
    {
        return 0U;
    }

    if ((strstr(payload, "\"result\":\"exit\"") != NULL) ||
        (strstr(payload, "\"status\":\"offline\"") != NULL) ||
        (strstr(payload, "\"status\":\"disconnect\"") != NULL))
    {
        return 1U;
    }

    return 0U;
}

/* 解析 RESULT JSON，只关心 result/status 字段，避免在 STM32 上引入完整 JSON 库。 */
BuzzerEvent_t GameProtocol_ParseResult(const char *payload)
{
    if (payload == NULL)
    {
        return BUZZER_EVENT_NONE;
    }

    if (strstr(payload, "\"result\":\"win\"") != NULL)
    {
        return BUZZER_EVENT_WIN;
    }

    if (strstr(payload, "\"result\":\"lose\"") != NULL)
    {
        return BUZZER_EVENT_LOSE;
    }

    if ((strstr(payload, "\"result\":\"reset\"") != NULL) ||
        (GameProtocol_IsExitResult(payload) != 0U))
    {
        return BUZZER_EVENT_RESET_SCORE;
    }

    return BUZZER_EVENT_NONE;
}

/* 解析 MQTT 剩余长度字段，当前支持标准变长编码。 */
static uint8_t MQTT_ReadRemainingLength(uint8_t *data, uint16_t len, uint16_t *value, uint16_t *used)
{
    uint16_t multiplier = 1U;
    uint16_t result = 0U;
    uint16_t index = 1U;
    uint8_t encoded;

    do
    {
        if (index >= len)
        {
            return 0U;
        }

        encoded = data[index++];
        result += (uint16_t)(encoded & 0x7FU) * multiplier;
        multiplier *= 128U;
    } while ((encoded & 0x80U) != 0U);

    *value = result;
    *used = index - 1U;
    return 1U;
}

/* 从 MQTT PUBLISH 原始包中提取本玩家 RESULT payload。 */
static uint8_t GameProtocol_ExtractResultPayload(uint8_t *data,
                                                 uint16_t len,
                                                 char *payload,
                                                 uint16_t payload_size)
{
    uint16_t remain_len = 0U;
    uint16_t remain_used = 0U;
    uint16_t pos;
    uint16_t topic_len;
    uint16_t payload_len;
    char topic[64];

    if ((data == NULL) || (payload == NULL) || (payload_size == 0U) || (len < 5U))
    {
        return 0U;
    }

    if ((data[0] & 0xF0U) != 0x30U)
    {
        return 0U;
    }

    if (MQTT_ReadRemainingLength(data, len, &remain_len, &remain_used) == 0U)
    {
        return 0U;
    }

    pos = (uint16_t)(1U + remain_used);
    if ((pos + 2U) > len)
    {
        return 0U;
    }

    topic_len = ((uint16_t)data[pos] << 8) | data[pos + 1U];
    pos += 2U;

    if ((topic_len >= sizeof(topic)) || ((pos + topic_len) > len))
    {
        return 0U;
    }

    memcpy(topic, &data[pos], topic_len);
    topic[topic_len] = '\0';
    pos += topic_len;

    if (strcmp(topic, TOPIC_RESULT) != 0)
    {
        return 0U;
    }

    payload_len = (uint16_t)(len - pos);
    if (payload_len >= payload_size)
    {
        payload_len = payload_size - 1U;
    }

    memcpy(payload, &data[pos], payload_len);
    payload[payload_len] = '\0';
    return 1U;
}

/* 解析 MQTT 原始 PUBLISH 包，如果是本玩家 RESULT 主题则返回蜂鸣器事件。 */
BuzzerEvent_t GameProtocol_ParseMqttPacket(uint8_t *data, uint16_t len)
{
    char payload[160];

    if (GameProtocol_ExtractResultPayload(data, len, payload, sizeof(payload)) == 0U)
    {
        return BUZZER_EVENT_NONE;
    }

    return GameProtocol_ParseResult(payload);
}

/* 判断 MQTT 原始 PUBLISH 包是否表示游戏退出或设备离线。 */
uint8_t GameProtocol_IsExitMqttPacket(uint8_t *data, uint16_t len)
{
    char payload[160];

    if (GameProtocol_ExtractResultPayload(data, len, payload, sizeof(payload)) == 0U)
    {
        return 0U;
    }

    return GameProtocol_IsExitResult(payload);
}
