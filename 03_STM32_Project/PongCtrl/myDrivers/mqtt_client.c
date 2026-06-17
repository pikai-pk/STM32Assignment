#include "mqtt_client.h"
#include "bsp_es8266.h"
#include "mqtt_config.h"
#include <string.h>
#include <stdio.h>

static uint32_t keep_alive_tick = 0;

/* MQTT 3.1.1 基础客户端。
 * 连接参数、主题和玩家编号统一从 app_config.h/mqtt_config.h 读取。
 */

/* 发送 MQTT CONNECT 报文，连接到 app_config.h 中配置的 Broker。 */
uint8_t MQTT_Connect(void)
{
    uint8_t buf[256];
    uint8_t resp[16] = {0};
    int idx = 0;

    // 固定头 CONNECT
    buf[idx++] = 0x10;

    // 剩余长度位置
    int len_pos = idx++;

    // 协议名 MQTT
    buf[idx++] = 0x00; buf[idx++] = 0x04;
    buf[idx++] = 'M'; buf[idx++] = 'Q'; buf[idx++] = 'T'; buf[idx++] = 'T';

    // 版本 4 (MQTT 3.1.1)
    buf[idx++] = 0x04;

    // 标志位：用户名+密码
    uint8_t connect_flags = 0x02;
    if (strlen(MQTT_USERNAME) > 0U)
    {
        connect_flags |= 0x80;
    }
    if (strlen(MQTT_PASSWORD) > 0U)
    {
        connect_flags |= 0x40;
    }
    buf[idx++] = connect_flags;

    // 心跳 60秒
    buf[idx++] = (uint8_t)(MQTT_KEEPALIVE_SECONDS >> 8);
    buf[idx++] = (uint8_t)(MQTT_KEEPALIVE_SECONDS & 0xFF);

    // 1. ClientID → deviceKey
    int len = strlen(MQTT_CLIENT_ID);
    buf[idx++] = len >> 8;
    buf[idx++] = len & 0xFF;
    memcpy(&buf[idx], MQTT_CLIENT_ID, len); idx += len;

    // 2. Username → AccessToken
    if (strlen(MQTT_USERNAME) > 0U)
    {
        len = strlen(MQTT_USERNAME);
        buf[idx++] = len >> 8;
        buf[idx++] = len & 0xFF;
        memcpy(&buf[idx], MQTT_USERNAME, len); idx += len;
    }

    // 3. Password → ProjectKey
    if (strlen(MQTT_PASSWORD) > 0U)
    {
        len = strlen(MQTT_PASSWORD);
        buf[idx++] = len >> 8;
        buf[idx++] = len & 0xFF;
        memcpy(&buf[idx], MQTT_PASSWORD, len); idx += len;
    }

    // 填充剩余长度
    buf[len_pos] = idx - 2;

    // 发送连接包
    ESP_SendRaw(buf, idx);
    keep_alive_tick = osKernelGetTickCount();

    osDelay(500);

    uint16_t recv_len = ESP_RecvRaw(resp, sizeof(resp));
    if (recv_len >= 4 && resp[0] == 0x20 && resp[1] == 0x02)
    {
        if (resp[3] == 0x00)
        {
            printf("MQTT CONNACK OK\r\n");
            return 0;
        }

        printf("MQTT CONNACK refused, code=%d\r\n", resp[3]);
        return 2;
    }

    printf("MQTT CONNACK timeout/no response, len=%d\r\n", recv_len);
    return 1;
}

/* 发布一条 QoS0 MQTT 消息，topic 和 payload 都使用字符串格式。 */
uint8_t MQTT_Publish(char *topic, char *payload)
{
    uint8_t buf[256];
    int idx = 0;
    buf[idx++] = 0x30;
    int topic_len = strlen(topic);
    int payload_len = strlen(payload);
    int remain = 2 + topic_len + payload_len;
    buf[idx++] = remain;
    buf[idx++] = topic_len >> 8;
    buf[idx++] = topic_len & 0xFF;
    memcpy(&buf[idx], topic, topic_len); idx += topic_len;
    memcpy(&buf[idx], payload, payload_len); idx += payload_len;
    ESP_SendRaw(buf, idx);
    return 0;
}

/* 订阅一个 MQTT 主题，当前使用固定 Packet ID=1 和 QoS0。 */
uint8_t MQTT_Subscribe(char *topic)
{
    uint8_t buf[128];
    int idx = 0;
    buf[idx++] = 0x82;
    int len = strlen(topic);
    buf[idx++] = 5 + len;
    buf[idx++] = 0x00; buf[idx++] = 0x01;
    buf[idx++] = len >> 8; buf[idx++] = len & 0xFF;
    memcpy(&buf[idx], topic, len); idx += len;
    buf[idx++] = 0x00;
    ESP_SendRaw(buf, idx);
    return 0;
}

// ===================== 30秒心跳 =====================
//void MQTT_KeepAlive(void)
//{
//    if (osKernelGetTickCount() - keep_alive_tick > 30000)
//    {
//        uint8_t ping[] = {0xC0, 0x00};
//        ESP_SendRaw(ping, 2);
//        keep_alive_tick = osKernelGetTickCount();
//    }
//}

/* 定期发送 MQTT PINGREQ，保持 Broker 连接不断开。 */
void MQTT_KeepAlive(void)
{
    // 30秒发一次PINGREQ
    if (osKernelGetTickCount() - keep_alive_tick > (MQTT_KEEPALIVE_SECONDS * 500U))
    {
        // 正确MQTT心跳包
        uint8_t ping_buf[2] = {0xC0, 0x00};
        ESP_SendRaw(ping_buf, 2);  // 发送心跳
        
        keep_alive_tick = osKernelGetTickCount();
        printf("MQTT heart beat package send ok\r\n");  // 调试看心跳MQTT 心跳发送成功
    }
}

/* 旧测试函数：解析 light_state 字段，当前 Pong 主流程不再使用。 */
void MQTT_ParseRecv(uint8_t *data, uint16_t len, uint8_t *light_state)
{
    char *p = strstr((char*)data, "\"light_state\"");
    if (!p) return;

    p = strchr(p, ':');
    if (!p) return;

    p++;
    while (*p == ' ') p++;

    if (!strncmp(p, "true", 4))  *light_state = 1;
    if (!strncmp(p, "false", 5)) *light_state = 0;
}






