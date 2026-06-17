#ifndef __KEYS_H__
#define __KEYS_H__

#include "main.h"

/* 根据电路设计，KEY1~KEY4按下，对应接口是低电平  */
#define KEY_DOWN 0
#define KEY_UP   1

uint8_t key1_scan(void);
uint8_t key2_scan(void);
uint8_t key1_is_pressed(void);
uint8_t key2_is_pressed(void);
uint8_t sw1_is_pressed(void);
uint8_t sw4_is_pressed(void);

#endif
