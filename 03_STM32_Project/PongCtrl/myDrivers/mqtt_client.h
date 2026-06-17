#ifndef __MQTT_CLIENT_H
#define __MQTT_CLIENT_H

#include "stm32f4xx_hal.h"

uint8_t MQTT_Connect(void);
uint8_t MQTT_Publish(char *topic, char *payload);
uint8_t MQTT_Subscribe(char *topic);
void MQTT_KeepAlive(void);
void MQTT_ParseRecv(uint8_t *data, uint16_t len, uint8_t *light_state);

#endif
