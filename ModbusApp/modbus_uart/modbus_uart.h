#pragma once

#include <furi.h>
#include "../Modbus.h"

LL_USART_InitTypeDef buildUartSettings(Config* cfg);
void uart_set_config(void* context);
void Serial_Begin(FuriHalSerialHandle* handle, LL_USART_InitTypeDef USART_InitStruct);
int32_t uart_worker(void* context);
void timerDone(void* context);
void serial_deinit(Uart* uart);
void serial_init(Uart* uart, uint8_t uart_ch);
