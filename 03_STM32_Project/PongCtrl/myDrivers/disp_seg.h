#ifndef __DISP_SEG_H__
#define __DISP_SEG_H__

#include "main.h"

/* 四位数码管驱动：
 * 74HC595 负责段选输出，74HC138 负责位选输出。
 * 当前工程优先使用 CubeMX 生成的 A0/A1/A2/A3/SCK/SER/DISLK/DISEN 引脚宏。
 * 如果工程中暂时没有这些引脚标签，下方提供默认映射，方便先通过编译。
 */
#ifndef A0_Pin
#define A0_Pin LED1_Pin
#define A0_GPIO_Port LED1_GPIO_Port
#endif

#ifndef A1_Pin
#define A1_Pin LED2_Pin
#define A1_GPIO_Port LED2_GPIO_Port
#endif

#ifndef A2_Pin
#define A2_Pin LED3_Pin
#define A2_GPIO_Port LED3_GPIO_Port
#endif

#ifndef A3_Pin
#define A3_Pin LED4_Pin
#define A3_GPIO_Port LED4_GPIO_Port
#endif

#ifndef SCK_Pin
#define SCK_Pin LED5_Pin
#define SCK_GPIO_Port LED5_GPIO_Port
#endif

#ifndef SER_Pin
#define SER_Pin LED6_Pin
#define SER_GPIO_Port LED6_GPIO_Port
#endif

#ifndef DISLK_Pin
#define DISLK_Pin LED7_Pin
#define DISLK_GPIO_Port LED7_GPIO_Port
#endif

void SEG_SendByte(uint8_t byte);
void SEG_Init(void);
void SEG_SelectBit(uint8_t bit);
void SEG_DisplayOne(uint8_t bit, uint8_t num);
void SEG_DisplayScan(uint16_t num);

#endif
