#include "led.h"

static void Led_Write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
    HAL_GPIO_WritePin(port, pin, state);
}

/* 初始化 LED 状态，默认全部熄灭。 */
void Led_Init(void)
{
    Led_AllOff();
}

/* 点亮 LED1~LED8。 */
void Led_AllOn(void)
{
    Led_Write(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    Led_Write(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    Led_Write(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
    Led_Write(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);
    Led_Write(LED5_GPIO_Port, LED5_Pin, GPIO_PIN_RESET);
    Led_Write(LED6_GPIO_Port, LED6_Pin, GPIO_PIN_RESET);
    Led_Write(LED7_GPIO_Port, LED7_Pin, GPIO_PIN_RESET);
    Led_Write(LED8_GPIO_Port, LED8_Pin, GPIO_PIN_RESET);
}

/* 熄灭 LED1~LED8。 */
void Led_AllOff(void)
{
    Led_Write(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    Led_Write(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    Led_Write(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
    Led_Write(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);
    Led_Write(LED5_GPIO_Port, LED5_Pin, GPIO_PIN_SET);
    Led_Write(LED6_GPIO_Port, LED6_Pin, GPIO_PIN_SET);
    Led_Write(LED7_GPIO_Port, LED7_Pin, GPIO_PIN_SET);
    Led_Write(LED8_GPIO_Port, LED8_Pin, GPIO_PIN_SET);
}
