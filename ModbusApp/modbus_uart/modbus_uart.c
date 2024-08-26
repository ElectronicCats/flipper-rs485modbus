#include "modbus_uart.h"
#include "../modbus_sender/modbus_sender.h"
#include "../modbus_parser/modbus_parser.h"

LL_USART_InitTypeDef buildUartSettings(Config* cfg) {
    LL_USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;

    USART_InitStruct.BaudRate = atoi(baudrateValues[cfg->baudrate]);
    USART_InitStruct.DataWidth =
        (cfg->dataWidth == 0 ? LL_USART_DATAWIDTH_7B :
         cfg->dataWidth == 2 ? LL_USART_DATAWIDTH_9B :
                               LL_USART_DATAWIDTH_8B);

    USART_InitStruct.StopBits =
        (cfg->stopBits == 0 ? LL_USART_STOPBITS_0_5 :
         cfg->stopBits == 2 ? LL_USART_STOPBITS_1_5 :
         cfg->stopBits == 3 ? LL_USART_STOPBITS_2 :
                              LL_USART_STOPBITS_1);

    USART_InitStruct.Parity =
        (cfg->parity == 1 ? LL_USART_PARITY_EVEN :
         cfg->parity == 2 ? LL_USART_PARITY_ODD :
                            LL_USART_PARITY_NONE);

    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = FURI_HAL_SERIAL_USART_OVERSAMPLING;
    return USART_InitStruct;
}
void uart_set_config(void* context) {
    furi_assert(context);
    App* app = context;
    UNUSED(app);
    // furi_thread_flags_set(furi_thread_get_id(app->uart->rxThread),
    // WorkerEvtCfgChange);
}
void Serial_Begin(FuriHalSerialHandle* handle, LL_USART_InitTypeDef USART_InitStruct) {
    furi_hal_bus_enable(FuriHalBusUSART1);
    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);

    furi_hal_gpio_init_ex(
        &gpio_usart_tx,
        GpioModeAltFunctionPushPull,
        GpioPullUp,
        GpioSpeedVeryHigh,
        GpioAltFn7USART1);
    furi_hal_gpio_init_ex(
        &gpio_usart_rx,
        GpioModeAltFunctionPushPull,
        GpioPullUp,
        GpioSpeedVeryHigh,
        GpioAltFn7USART1);
    LL_USART_Init(USART1, &USART_InitStruct);
    LL_USART_EnableFIFO(USART1);
    LL_USART_ConfigAsyncMode(USART1);

    LL_USART_Enable(USART1);

    while(!LL_USART_IsActiveFlag_TEACK(USART1) || !LL_USART_IsActiveFlag_REACK(USART1))
        ;

    furi_hal_serial_set_br(handle, USART_InitStruct.BaudRate);
    LL_USART_DisableIT_ERROR(USART1);
}

static void
    on_rx_cb(FuriHalSerialHandle* handle, FuriHalSerialRxEvent ev, size_t size, void* context) {
    Uart* uart = context;
    if(ev & (FuriHalSerialRxEventData | FuriHalSerialRxEventIdle)) {
        uint8_t data[FURI_HAL_SERIAL_DMA_BUFFER_SIZE] = {0};
        while(size) {
            size_t ret = furi_hal_serial_dma_rx(
                handle,
                data,
                (size > FURI_HAL_SERIAL_DMA_BUFFER_SIZE) ? FURI_HAL_SERIAL_DMA_BUFFER_SIZE : size);
            furi_stream_buffer_send(uart->rxStream, data, ret, 0);
            size -= ret;
        };
        furi_thread_flags_set(furi_thread_get_id(uart->rxThread), WorkerEvtRxDone);
    }
}
void serial_init(Uart* uart, uint8_t uart_ch) {
    furi_assert(!uart->serial_handle);
    uart->serial_handle = furi_hal_serial_control_acquire(uart_ch);
    furi_assert(uart->serial_handle);

    Serial_Begin(uart->serial_handle, buildUartSettings(uart->cfg));
    furi_hal_serial_dma_rx_start(uart->serial_handle, on_rx_cb, uart, false);
}
void serial_deinit(Uart* uart) {
    if(uart->serial_handle == NULL) {
        return;
    }
    furi_hal_serial_dma_rx_stop(uart->serial_handle);
    furi_hal_serial_deinit(uart->serial_handle);
    furi_hal_serial_control_release(uart->serial_handle);
    uart->serial_handle = NULL;
}
void timerDone(void* context) {
    App* app = context;
    app->modbus->slave = false;
}
int32_t uart_worker(void* context) {
    App* app = context;
    while(1) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        furi_check((events & FuriFlagError) == 0);
        if(events & WorkerEvtStop) break;
        if(events & WorkerEvtCfgChange) {
            serial_deinit(app->uart);
            serial_init(app->uart, UART_CH);
        }
        if(events & WorkerEvtRxDone) {
            size_t len =
                furi_stream_buffer_receive(app->uart->rxStream, app->uart->rxBuff, RX_BUF_SIZE, 0);
            if(len > 0) {
                handle_rx_data_cb(app->uart->rxBuff, len, app);
            }
        }
        if(events & WorkerEvtTxStart) {
            ModbusSender(app);
        }
    }

    return 0;
}
