#pragma once

#include <furi.h>

void handle_rx_data_cb(uint8_t* buf, size_t len, void* context);
uint16_t getCRC(uint8_t* buf, uint8_t len);
