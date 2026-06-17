#include "bsp_uart_fifo.h"

UART_FIFO_T uart6_fifo;
extern UART_HandleTypeDef huart6;
static uint8_t uart6_rx_byte;
	
/* 初始化指定串口的环形缓冲区，并清空读写指针。 */
void UART_FIFO_Init(UART_FIFO_T *fifo, UART_HandleTypeDef *huart) {
    fifo->huart = huart;
    fifo->w = fifo->r = 0;
    memset(fifo->buf, 0, UART_FIFO_SIZE);
}

/* 把 USART6 中断收到的 1 字节数据写入环形缓冲区。 */
void UART_FIFO_Write(UART_FIFO_T *fifo, uint8_t data) {
    fifo->buf[fifo->w] = data;
    fifo->w = (fifo->w + 1) % UART_FIFO_SIZE;
}

/* 获取环形缓冲区中当前可读取的数据长度。 */
uint16_t UART_FIFO_Len(UART_FIFO_T *fifo) {
    return (UART_FIFO_SIZE + fifo->w - fifo->r) % UART_FIFO_SIZE;
}

/* 从环形缓冲区读取指定长度的数据，返回实际读取字节数。 */
uint16_t UART_FIFO_Read(UART_FIFO_T *fifo, uint8_t *buf, uint16_t len) {
    uint16_t i = 0;
    while(UART_FIFO_Len(fifo) && i < len) {
        buf[i++] = fifo->buf[fifo->r];
        fifo->r = (fifo->r + 1) % UART_FIFO_SIZE;
    }
    return i;
}

/* 开启 USART6 单字节中断接收，用于持续接收 ESP8266 返回数据。 */
void UART_Start_IT(UART_FIFO_T *fifo) {
    HAL_UART_Receive_IT(fifo->huart, &uart6_rx_byte, 1);
}

/* HAL 串口接收完成回调：USART6 数据写入 FIFO，并立即开启下一次接收。 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart->Instance == USART6) {
        UART_FIFO_Write(&uart6_fifo, uart6_rx_byte);
        //UART_Start_IT(&uart6_fifo);
				// 重新开启中断
        HAL_UART_Receive_IT(&huart6, &uart6_rx_byte, 1); 
    }
    if(huart->Instance == USART1) {
        // 调试口
    }
}
