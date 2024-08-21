#include "../Modbus.h"
#include "../modbus_parser/modbus_parser.h"

void OnItemEnterCB(void* context, uint32_t index) {
    App* app = context;
    uint8_t start = index ? app->ringBuffer->delimiters[index - 1] + 1 : 0;
    for(uint8_t i = start; i <= app->ringBuffer->delimiters[index]; i++) {
        app->msgBuf[i - start] = app->ringBuffer->ringBuffer[i];
        if(i == app->ringBuffer->delimiters[index]) app->msgLen = i - start + 1;
    }
    scene_manager_next_scene(app->sceneManager, app_scene_manual_sender);
}
void BuildCMDList(App* app) {
    submenu_set_header(app->subMenu, " ID | FN |Adss");
    RingBuffer* rb = app->ringBuffer;
    rb->readIdx = 0;
    uint8_t buf[255];
    uint8_t i = 0;
    uint8_t delimiterIdx = 0;
    uint8_t len = 0;
    char lbl[30];
    do {
        len = 0;
        do {
            snprintf(lbl, sizeof(lbl), " %d | %d | %d |", SLAVE, FUNCTION, STARTADDRESS);
            buf[len] = rb->ringBuffer[i];
            len++;
            i++;
        } while(i <= rb->delimiters[delimiterIdx] && i < 255);
        delimiterIdx++;
        if((CRCH | CRCL << 8) == getCRC(buf, len - 2))
            submenu_add_item(app->subMenu, lbl, delimiterIdx - 1, OnItemEnterCB, app);
    } while(i < 255);
}
void app_scene_msgs_buffer_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->subMenu);
    BuildCMDList(app);
    view_dispatcher_switch_to_view(app->viewDispatcher, Submenu_View);
}
bool app_scene_msgs_buffer_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}
void app_scene_msgs_buffer_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->subMenu);
}
