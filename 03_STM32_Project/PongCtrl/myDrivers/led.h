#ifndef __LED_H__
#define __LED_H__

#include "main.h"

/* 板载 LED1~LED8 使用低电平点亮，高电平熄灭。 */
void Led_Init(void);
void Led_AllOn(void);
void Led_AllOff(void);

#endif
