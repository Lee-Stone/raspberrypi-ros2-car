#ifndef __SERIAL_H
#define __SERIAL_H

#include "config.h"

#if DEVICE_SERIAL

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    UART_HandleTypeDef  *huart;     /* 关联的 UART 句柄（由 Serial_Init 写入）    */
    uint8_t              rxByte;    /* HAL 中断接收的单字节暂存区                  */
    uint8_t              rxData;    /* 最近一次中断收到的有效字节                  */
    volatile uint8_t     rxFlag;    /* 接收标志位：中断置 1，Serial_Read 清 0      */
} Serial;

void Serial_Init(Serial *serial, UART_HandleTypeDef *huart);
void Serial_Printf(Serial *serial, const char *fmt, ...);
bool Serial_Received(Serial *serial);
unsigned char Serial_Read(Serial *serial);

#endif /* DEVICE_SERIAL */

#endif /* __SERIAL_H */
