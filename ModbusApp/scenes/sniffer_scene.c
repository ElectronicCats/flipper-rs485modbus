#include "../Modbus.h"
#include "../modbus_uart/modbus_uart.h"
#include "../modbus_storage/modbus_storage.h"

static void sniffer_begin(App* app) {
    serial_init(app->uart, UART_CH);
    if(app->uart->cfg->saveLOG && !storage_file_is_open(app->LOGfile)) {
        open_log_file_stream(app);
    }
}
void app_scene_sniffer_on_enter(void* context) {
    App* app = context;
    if(scene_manager_get_scene_state(app->sceneManager, app_scene_sniffer) == Sniffer_Option ||
       scene_manager_get_scene_state(app->sceneManager, app_scene_sniffer) == Sender_Option) {
        sniffer_begin(app);
        text_box_set_font(app->textBox, TextBoxFontText);
        text_box_set_focus(app->textBox, TextBoxFocusEnd);
        furi_string_cat_printf(
            app->text, "Baudrate: %s", baudrateValues[app->uart->cfg->baudrate]);
        furi_string_cat_printf(
            app->text, "\nData Width: %s", dataWidthValues[app->uart->cfg->dataWidth]);
        furi_string_cat_printf(
            app->text, "\nStop bits: %s", stopBitsValues[app->uart->cfg->stopBits]);
        furi_string_cat_printf(app->text, "\nParity: %s", parityValues[app->uart->cfg->parity]);
        furi_string_cat_printf(
            app->text, "\nResponse TimeOut: %dms", app->uart->cfg->timeout * TIMEOUT_SCALER);
    } else if(scene_manager_get_scene_state(app->sceneManager, app_scene_sniffer) == About_Option) {
        text_box_set_font(app->textBox, TextBoxFontText);
        text_box_set_focus(app->textBox, TextBoxFocusStart);
        furi_string_cat_printf(app->text, "MODBUS APP\n");
        furi_string_cat_printf(app->text, "BY: ROBERTO ARELLANO\n");
        furi_string_cat_printf(app->text, "ELECTRONIC CATS\n");
        furi_string_cat_printf(
            app->text,
            "https://github.com/ElectronicCats/"
            "flipper-rs485modbus/tree/main/ModbusApp/Test1");
    }
    view_dispatcher_switch_to_view(app->viewDispatcher, TextBox_View);
    text_box_set_text(app->textBox, furi_string_get_cstr(app->text));
    if(scene_manager_get_scene_state(app->sceneManager, app_scene_sniffer) == Sender_Option) {
        furi_thread_flags_set(furi_thread_get_id(app->uart->rxThread), WorkerEvtTxStart);
    }
}
bool app_scene_sniffer_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    UNUSED(app);
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        text_box_set_text(app->textBox, furi_string_get_cstr(app->text));
    }
    return consumed;
}
void app_scene_sniffer_on_exit(void* context) {
    App* app = context;
    text_box_reset(app->textBox);
    furi_string_reset(app->text);
    serial_deinit(app->uart);
}
