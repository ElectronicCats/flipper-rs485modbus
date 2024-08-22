#pragma once

#include <furi.h>

#include "../Modbus.h"

RingBuffer* ring_buffer_alloc();
void ring_buffer_free(RingBuffer* buffer);
void writeRingBuffer(RingBuffer* rb, uint8_t* buf, size_t len);
