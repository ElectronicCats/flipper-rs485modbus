#include "modbus_sender.h"
#include "../modbus_parser/modbus_parser.h"

void ModbusSender(void* context) {
    App* app = context;
    // 02 | 0F | 00 00 | 00 04 | 01 | 0C | 7E 86
    Uart* uart = app->uart;
    uint16_t crc = getCRC(app->msgBuf, app->msgLen - 2);
    app->msgBuf[app->msgLen - 2] = crc & 0x00FF;
    app->msgBuf[app->msgLen - 1] = (crc & 0xFF00) >> 8;
    furi_hal_gpio_write(&gpio_ext_pc0, true);
    furi_hal_gpio_write(&gpio_ext_pc1, true);
    furi_hal_serial_tx(uart->serial_handle, app->msgBuf, app->msgLen);
    furi_hal_serial_tx_wait_complete(uart->serial_handle);
    furi_hal_gpio_write(&gpio_ext_pc0, false);
    furi_hal_gpio_write(&gpio_ext_pc1, false);
    app->modbus->slave = true;
    furi_timer_start(app->timer, app->uart->cfg->timeout * TIMEOUT_SCALER);
}
