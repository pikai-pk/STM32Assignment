#ifndef __BSP_UART_FIFO_H
#define __BSP_UART_FIFO_H

// 串口接收环形缓冲区，防止中断接收丢包

#include "stm32f4xx_hal.h"
#include <string.h>

#define UART_FIFO_SIZE 1024

typedef struct {
    UART_HandleTypeDef *huart;
    uint8_t buf[UART_FIFO_SIZE];
    volatile uint16_t w;
    volatile uint16_t r;
}UART_FIFO_T;

extern UART_FIFO_T uart6_fifo;

void UART_FIFO_Init(UART_FIFO_T *fifo, UART_HandleTypeDef *huart);
void UART_FIFO_Write(UART_FIFO_T *fifo, uint8_t data);
uint16_t UART_FIFO_Read(UART_FIFO_T *fifo, uint8_t *buf, uint16_t len);
uint16_t UART_FIFO_Len(UART_FIFO_T *fifo);
void UART_Start_IT(UART_FIFO_T *fifo);

#endif
