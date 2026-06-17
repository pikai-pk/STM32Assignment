#include "usart.h"
#include <stdio.h>

#if defined(__CC_ARM)
#pragma import(__use_no_semihosting)
struct __FILE
{
    int handle;
};
FILE __stdout;

/* 关闭 Keil 半主机退出通道，避免未连接调试器时卡死。 */
void _sys_exit(int x)
{
    (void)x;
}
#endif

/* 将 printf 重定向到 USART1，方便在串口助手中观察 Wi-Fi/MQTT/按键日志。 */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1U, 100U);
    return ch;
}
