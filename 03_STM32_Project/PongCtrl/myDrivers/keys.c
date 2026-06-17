/*
定义按键的核心操作
	判断按键是否按下，如果按下返回1， 否则返回0

*/

#include "main.h"
#include "keys.h"
#include "cmsis_os2.h"

/* 直接读取 SW1(PE1) 当前电平：按下返回 1，松开返回 0；用于 FreeRTOS 高频轮询。 */
uint8_t sw1_is_pressed(void)
{
	return (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET) ? 1U : 0U;
}

/* 直接读取 SW4(PE4) 当前电平：按下返回 1，松开返回 0；用于 FreeRTOS 高频轮询。 */
uint8_t sw4_is_pressed(void)
{
	return (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_RESET) ? 1U : 0U;
}

uint8_t key1_is_pressed(void)
{
	return sw1_is_pressed();
}

uint8_t key2_is_pressed(void)
{
	return sw4_is_pressed();
}

/* 按键低电平表示按下。
 * KEY1 用作左移，KEY2 用作右移。
 */

/* 扫描 KEY1，确认低电平稳定后返回 KEY_DOWN。 */
uint8_t key1_scan(void)
{
	uint8_t a = KEY_UP;
	if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET) {						//检测按键是否按下
			osDelay(10);
			if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET) {				
//				while(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET);			//等待按键松开
				a = KEY_DOWN;
			}
		}
	return a;
}

/* 扫描 KEY2，确认低电平稳定后返回 KEY_DOWN。 */
uint8_t key2_scan(void)
{
	uint8_t a = KEY_UP;
	if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET) {
			osDelay(10);
			if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET) {
				a = KEY_DOWN;
			}
		}
	return a;
}

//uint8_t key2_scan(void)
//{
//	uint8_t a = KEY_UP;
//	if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET) {						//检测按键是否按下
//			HAL_Delay(10);
//			if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET) {				
//				while(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET);			//等待按键松开
//				a = KEY_DOWN;
//			}
//		}
//	return a;
//}

