#include "modbus_ring_buffer.h"

RingBuffer* ring_buffer_alloc() {
    RingBuffer* buffer = malloc(sizeof(RingBuffer));
    buffer->writeIdx = 0;
    buffer->delimiterIdx = 0;
    for(uint8_t i = 0; i < 32; i++)
        buffer->delimiters[i] = 255;
    return buffer;
}
void ring_buffer_free(RingBuffer* buffer) {
    free(buffer);
}
void writeRingBuffer(RingBuffer* rb, uint8_t* buf, size_t len) {
    for(size_t i = 0; i < len; i++) {
        rb->ringBuffer[rb->writeIdx] = buf[i];
        if(i == len - 1) rb->delimiters[rb->delimiterIdx] = rb->writeIdx;
        if(++rb->writeIdx > 255) {
            rb->delimiterIdx = 0;
            rb->writeIdx = 0;
        }
    }
    rb->delimiterIdx++;
}
