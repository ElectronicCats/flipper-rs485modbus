#include "modbus_parser.h"

#include "../Modbus.h"
#include "../modbus_ring_buffer/modbus_ring_buffer.h"

uint16_t getCRC(uint8_t* buf, uint8_t len) {
    uint16_t crc = 0xFFFF;

    for(int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];

        for(int i = 8; i != 0; i--) {
            if((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else
                crc >>= 1;
        }
    }
    return crc;
}

static void discreteValuesParser(void* context, uint8_t* buff, size_t len, FuriString* data) {
    App* app = context;
    uint8_t value = 0;
    uint8_t offset = 0;
    while(len) {
        memcpy(&value, buff + offset, 1);
        offset++;
        if(!app->uart->cfg->hexOutput) {
            furi_string_cat_printf(data, "\n-Byte%d: \n->", offset);
            for(int i = 0; i < 8; i++)
                furi_string_cat_printf(
                    data,
                    "%s%s",
                    value >> i & 0x01 ? "ON" : "OFF",
                    i == 3 ? "\n->" :
                    i == 7 ? "" :
                             ",");
        } else
            furi_string_cat_printf(data, "\n->Byte%d: 0x%02X", offset, value);
        len--;
    }
}

static void analogValuesParser(void* context, uint8_t* buff, size_t len, FuriString* data) {
    App* app = context;
    uint16_t value = 0;
    size_t offset = 0;

    while(offset < len) {
        value = 0;
        if(offset + 1 < len) {
            memcpy(((uint8_t*)&value) + 1, buff + offset, sizeof(uint8_t));
            memcpy((uint8_t*)&value, buff + offset + 1, sizeof(uint8_t));
        } else if(offset < len) {
            memcpy(((uint8_t*)&value) + 1, buff + offset, sizeof(uint8_t));
        }

        furi_string_cat_printf(
            data,
            app->uart->cfg->hexOutput ? "\n->Reg%d: 0x%04X" : "\n->Reg%d: %d",
            offset / 2,
            value);

        offset += 2;
    }
}

static void pduParser(void* context, bool slave, uint8_t* buf, size_t len, FuriString* data) {
    App* app = context;
    size_t offset = 2;
    uint16_t address = 0;
    uint16_t qty = 0;
    uint16_t bCount = 0;
    uint16_t value = 0;
    UNUSED(len);
    furi_string_cat_printf(
        data, "\n%s", functionNames[FUNCTION <= 6 ? FUNCTION - 1 : FUNCTION - 9]);
    furi_string_cat_printf(
        data, app->uart->cfg->hexOutput ? "\nPeripheral: 0x%02X" : "\nPeripheral: %d", SLAVE);
    memcpy(
        slave && FUNCTION <= 4 ? &bCount : &address, buf + offset, slave && FUNCTION <= 4 ? 1 : 2);

    offset += slave && FUNCTION <= 4 ? 1 : 2;
    address = address >> 8 | address << 8;
    if(app->uart->cfg->hexOutput)
        furi_string_cat_printf(
            data,
            slave && FUNCTION <= 4 ? "\nbCount: 0x%02X" : "\nAddress: 0x%04X",
            slave && FUNCTION <= 4 ? bCount : address);
    else
        furi_string_cat_printf(
            data,
            slave && FUNCTION <= 4 ? "\nbCount: %d" : "\nAddress: %d",
            slave && FUNCTION <= 4 ? bCount : address);

    if(FUNCTION >= 0x0F || (!slave && FUNCTION <= 0x04)) {
        memcpy(&qty, buf + offset, 2);
        offset += 2;
        qty = qty >> 8 | qty << 8;
        furi_string_cat_printf(
            data, app->uart->cfg->hexOutput ? "\nQty: 0x%04X" : "\nQty: %d", qty);
    } else if(FUNCTION >= 0x05) {
        memcpy(&value, buf + offset, 2);
        offset += 2;
        value = value >> 8 | value << 8;
        if(FUNCTION == 0x05)
            furi_string_cat_printf(data, "\nValue: %s", buf[4] ? "ON" : "OFF");
        else
            furi_string_cat_printf(
                data, app->uart->cfg->hexOutput ? "\nValue: 0x%04X" : "\nValue: %d", value);
    } else if(FUNCTION <= 0x02)
        discreteValuesParser(app, buf + offset, bCount, data);
    else
        analogValuesParser(app, buf + offset, bCount, data);

    if(FUNCTION >= 0x0F && !slave) {
        memcpy(&bCount, buf + offset, 1);
        offset++;
        furi_string_cat_printf(
            data, app->uart->cfg->hexOutput ? "\nbCount: 0x%02X" : "\nbCount: %d", bCount);
        if(FUNCTION == 0x0F)
            discreteValuesParser(app, buf + offset, bCount, data);
        else
            analogValuesParser(app, buf + offset, bCount, data);
    }
    furi_string_cat_printf(data, "\nCRC: 0x%02X", CRCL | CRCH << 8);
}
static void ErrParser(uint8_t* buf, size_t len, FuriString* data) {
    furi_string_cat_printf(
        data, "\nException code (%02X):\n%s\n", FUNCTION, exceptionCodes[EXCEPTION]);
    for(size_t i = 0; i < len; i++)
        furi_string_cat_printf(data, "%02X", buf[i]);
}
static void ModbusParser(uint8_t* buf, size_t len, App* app, FuriString* data) {
    if(FUNCTION > 0x80) {
        ErrParser(buf, len, data);
    } else if((FUNCTION > 0x06 && FUNCTION < 0x0F) || FUNCTION > 0x10) {
        furi_string_cat_printf(data, "\nUNSUPPORTED!!!\nFUNCTION(0x%02X)\n", FUNCTION);
        for(size_t i = 0; i < len; i++)
            furi_string_cat_printf(data, "%02X", buf[i]);
    } else if(FixedPaket && len - 4 != FixedModbusSize) {
        furi_string_cat_str(data, "\nLength-Type MissMatch!!!\n");
        for(size_t i = 0; i < len; i++)
            furi_string_cat_printf(data, "%02X", buf[i]);
        furi_string_cat_printf(
            data,
            "\nCheck Reponse TimeOut!!!\nCurrent: %dms",
            app->uart->cfg->timeout * TIMEOUT_SCALER);
    } else {
        if(!app->modbus->slave) {
            for(size_t i = 0; i < len; i++)
                app->msgBuf[i] = buf[i];
            writeRingBuffer(app->ringBuffer, buf, len);
            app->msgLen = len;
        }
        pduParser(app, app->modbus->slave, buf, len, data);
    }
}
void handle_rx_data_cb(uint8_t* buf, size_t len, void* context) {
    furi_assert(context);
    App* app = context;
    buf[len] = '\0';
    FuriString* data = furi_string_alloc();
    furi_string_reset(data);
    ///*
    furi_string_cat_printf(
        data, "\n-----%s----", app->modbus->slave ? "PERIPHERAL-" : "---HUB----");
    if((CRCH | CRCL << 8) == getCRC(buf, len - 2)) {
        ModbusParser(buf, len, app, data);
    } else {
        furi_string_cat_str(data, "\nCRC check Failed:\n");
        for(size_t i = 0; i < len; i++)
            furi_string_cat_printf(data, "%02X", buf[i]);
        furi_string_cat_str(data, "\nPlease check UART Settings!!!");
    }
    //*/
    // for(size_t i = 0; i < len; i++) furi_string_cat_printf(data, "%02X",
    // buf[i]); furi_string_cat_str(data, "\n");
    app->textLen += furi_string_size(data);
    if(app->textLen >= 3500 - 1) {
        furi_string_right(app->text, app->textLen / 2);
        app->textLen = furi_string_size(app->text) + furi_string_size(data);
    }
    furi_string_cat_str(app->text, furi_string_get_cstr(data));

    if(app->LOGfileReady)
        storage_file_write(app->LOGfile, furi_string_get_cstr(data), furi_string_size(data));

    furi_string_free(data);

    view_dispatcher_send_custom_event(app->viewDispatcher, Refresh);

    if(app->modbus->slave) {
        app->modbus->slave = false;
        furi_timer_stop(app->timer);
    } else {
        app->modbus->slave = true;
        furi_timer_start(app->timer, app->uart->cfg->timeout * TIMEOUT_SCALER);
    }
}
