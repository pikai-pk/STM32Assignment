#include "bsp_es8266.h"
#include "mqtt_config.h"
#include <stdio.h>
#include <string.h>


// 针对普通AT固件的wifi模块驱动

/* ESP8266 AT 固件驱动：
 * 1. 连接 app_config.h 中配置的 Wi-Fi 热点
 * 2. 连接 app_config.h 中配置的 MQTT Broker TCP 端口
 * 3. 进入透传模式后给 MQTT 客户端收发原始数据
 */

extern UART_FIFO_T uart6_fifo;

extern UART_HandleTypeDef huart6;

/* 退出 ESP8266 透传模式，并清空串口接收缓冲区。 */
static void ESP_ExitTransparentMode(void)
{
    osDelay(1200);
    HAL_UART_Transmit(&huart6, (uint8_t*)"+++", 3, 1000);
    osDelay(1200);

    memset(uart6_fifo.buf, 0, UART_FIFO_SIZE);
    uart6_fifo.w = 0;
    uart6_fifo.r = 0;
}

/* 发送一条 AT 指令，并在超时时间内等待指定应答字符串。 */
static uint8_t AT_Send(char *cmd, char *ack, uint32_t timeout)
{
    uint8_t buf[128] = {0};
    char acc[512] = {0};
    uint16_t acc_len = 0;
    uint32_t tick = osKernelGetTickCount();

    // 清空缓冲区
    memset(uart6_fifo.buf, 0, UART_FIFO_SIZE);
    uart6_fifo.w = 0;
    uart6_fifo.r = 0;

    // 发送指令（加长延时）
    HAL_UART_Transmit(&huart6, (uint8_t*)cmd, strlen(cmd), 1000);
    HAL_UART_Transmit(&huart6, (uint8_t*)"\r\n", 2, 1000);

    // 超时退出，不卡死
    while(osKernelGetTickCount() - tick < timeout)
    {
        int len = UART_FIFO_Read(&uart6_fifo, buf, sizeof(buf) - 1);
        if(len > 0)
        {
            buf[len] = 0;
            printf("AT RECV: %s\r\n", buf); // 调试打印

            if (acc_len + len >= sizeof(acc))
            {
                acc_len = 0;
                memset(acc, 0, sizeof(acc));
            }
            memcpy(&acc[acc_len], buf, len);
            acc_len += len;
            acc[acc_len] = 0;

            if(strstr(acc, ack))
                return 0;
        }
        osDelay(10);
    }

    printf("AT TIMEOUT: %s\r\n", cmd);
    return 1; // 超时退出，不卡死
}


/* 多次探测 AT，避免 ESP8266 刚复位、残留透传或忙状态时第一次 AT 失败。 */
static uint8_t ESP_WaitATReady(void)
{
    uint8_t i;

    for (i = 0U; i < 5U; i++)
    {
        printf("AT Test try %u...\r\n", (unsigned int)(i + 1U));
        if (AT_Send("AT", "OK", 1500U) == 0U)
        {
            return 0U;
        }

        ESP_ExitTransparentMode();
        osDelay(500U);
    }

    return 1U;
}

//static uint8_t AT_Send(char *cmd, char *ack, uint32_t timeout) {
//    uint8_t buf[512] = {0};
//    uint32_t tick = osKernelGetTickCount();
//    memset(uart6_fifo.buf, 0, UART_FIFO_SIZE);
//    uart6_fifo.w = uart6_fifo.r = 0;

//    HAL_UART_Transmit(&huart6, (uint8_t*)cmd, strlen(cmd), 1000);
//    HAL_UART_Transmit(&huart6, (uint8_t*)"\r\n", 2, 1000);

//    while(osKernelGetTickCount() - tick < timeout) {
//        if(UART_FIFO_Len(&uart6_fifo) > 0) {
//            UART_FIFO_Read(&uart6_fifo, buf, 500);
//            if(strstr((char*)buf, ack)) return 0;
//        }
//        osDelay(1);
//    }
//    return 1;
//}

/* 初始化 ESP8266：退出透传、测试 AT、设置 STA 模式并连接 Wi-Fi。 */
uint8_t ESP_Init(void)
{
    ESP_ExitTransparentMode();

    printf("AT Test...\r\n");
    if(ESP_WaitATReady()) return 1;

    /* 清理旧状态。失败也继续，避免残留 TCP/透传状态影响下一轮初始化。 */
    AT_Send("ATE0", "OK", 1000);
    AT_Send("AT+CIPCLOSE", "OK", 1000);
    AT_Send("AT+CIPMODE=0", "OK", 1000);

    printf("Set WiFi Mode...\r\n");
    if(AT_Send("AT+CWMODE=1", "OK", 2000)) return 2;

    printf("Reset...\r\n");
    if(AT_Send("AT+RST", "OK", 3000)) return 3;
    osDelay(5000);

    // 连WiFi
    char cmd[128];
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASSWORD);
    printf("Connecting WiFi...\r\n");

    // 这里等待10秒，超时也继续
    if(AT_Send(cmd, "OK", 20000) == 0)
    {
        printf("WiFi Connected OK!\r\n");
    }
    else
    {
        printf("WiFi Timeout\r\n");
        return 4;
    }

    if(AT_Send("AT+CIPMUX=0", "OK", 2000)) return 5;
    return 0;
}

/* 读取 ESP8266 当前 STA IP 地址，成功时写入 ip_buf，失败时写入 unknown。 */
uint8_t ESP_GetLocalIP(char *ip_buf, uint16_t buf_len)
{
    uint8_t buf[128] = {0};
    char acc[384] = {0};
    uint16_t acc_len = 0;
    uint32_t tick = osKernelGetTickCount();
    char *start;
    char *end;

    if ((ip_buf == NULL) || (buf_len == 0U))
    {
        return 1U;
    }

    snprintf(ip_buf, buf_len, "%s", "unknown");

    memset(uart6_fifo.buf, 0, UART_FIFO_SIZE);
    uart6_fifo.w = 0;
    uart6_fifo.r = 0;

    HAL_UART_Transmit(&huart6, (uint8_t*)"AT+CIFSR\r\n", 10, 1000);

    while(osKernelGetTickCount() - tick < 3000U)
    {
        int len = UART_FIFO_Read(&uart6_fifo, buf, sizeof(buf) - 1U);
        if(len > 0)
        {
            buf[len] = 0;

            if (acc_len + len >= sizeof(acc))
            {
                acc_len = 0;
                memset(acc, 0, sizeof(acc));
            }

            memcpy(&acc[acc_len], buf, len);
            acc_len += len;
            acc[acc_len] = 0;

            start = strstr(acc, "STAIP,\"");
            if (start != NULL)
            {
                start += strlen("STAIP,\"");
                end = strchr(start, '\"');
                if (end != NULL)
                {
                    uint16_t copy_len = (uint16_t)(end - start);
                    if (copy_len >= buf_len)
                    {
                        copy_len = buf_len - 1U;
                    }

                    memcpy(ip_buf, start, copy_len);
                    ip_buf[copy_len] = '\0';
                    return 0U;
                }
            }
        }

        osDelay(10U);
    }

    return 2U;
}

//uint8_t ESP_Init(void) {
//    AT_Send("AT", "OK", 1000);
//    AT_Send("AT+CWMODE=1", "OK", 1000);
//    AT_Send("AT+RST", "ready", 2000);
//    osDelay(1000);

//    char cmd[128];
//    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASS);
//    if(AT_Send(cmd, "OK", 10000)) return 1;

//    AT_Send("AT+CIPMUX=0", "OK", 1000);
//    return 0;
//}

/* 连接 MQTT Broker 的 TCP 端口，并进入 ESP8266 透传发送模式。 */
uint8_t ESP_ConnectMQTTServer(void) {
    char cmd[128];
    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d", MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    printf("ESP TCP START: %s:%d\r\n", MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    if(AT_Send(cmd, "OK", 10000))
    {
        printf("ESP TCP START failed\r\n");
        return 1;
    }

    printf("ESP set transparent mode...\r\n");
    if(AT_Send("AT+CIPMODE=1", "OK", 2000))
    {
        printf("ESP transparent mode failed\r\n");
        return 2;
    }

    printf("ESP enter send mode...\r\n");
    if(AT_Send("AT+CIPSEND", ">", 3000))
    {
        printf("ESP CIPSEND failed\r\n");
        return 3;
    }

    printf("ESP transparent send mode OK\r\n");
    return 0;
}

//void ESP_SendRaw(uint8_t *data, uint16_t len) {
//    HAL_UART_Transmit(&huart6, data, len, 100);
//}

/* 通过 ESP8266 透传模式发送 MQTT 原始字节流。 */
void ESP_SendRaw(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart6, data, len, 1000);
    osDelay(5);  // 加这一行，防止透传模式丢包
}

/* 从 ESP8266 接收 FIFO 中读取 MQTT 或 AT 返回数据。 */
uint16_t ESP_RecvRaw(uint8_t *buf, uint16_t len) {
    return UART_FIFO_Read(&uart6_fifo, buf, len);
}
