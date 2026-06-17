#ifndef __BSP_ESP8266_H
#define __BSP_ESP8266_H

// 针对普通AT固件的wifi模块驱动

#include "stm32f4xx_hal.h"
#include "bsp_uart_fifo.h"
#include "cmsis_os2.h"

/* ESP8266 AT 固件驱动接口。 */

uint8_t ESP_Init(void);
uint8_t ESP_GetLocalIP(char *ip_buf, uint16_t buf_len);
uint8_t ESP_ConnectMQTTServer(void);
void ESP_SendRaw(uint8_t *data, uint16_t len);
uint16_t ESP_RecvRaw(uint8_t *buf, uint16_t len);

#endif
