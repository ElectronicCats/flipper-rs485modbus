#include "../Modbus.h"

//////////////////////////   ByteInput Scene  ////////////////////////
void SetValue(void* context) {
    App* app = context;
    scene_manager_handle_back_event(app->sceneManager);
}
void app_scene_byte_input_on_enter(void* context) {
    App* app = context;
    uint8_t* buf = app->msgBuf;
    uint8_t offset = scene_manager_get_scene_state(app->sceneManager, app_scene_byte_input);
    switch(scene_manager_get_scene_state(app->sceneManager, app_scene_byte_input)) {
    case 0:
        byte_input_set_header_text(app->byteInput, "Set Slave");
        byte_input_set_result_callback(app->byteInput, SetValue, NULL, app, &SLAVE, 1);
        view_dispatcher_switch_to_view(app->viewDispatcher, ByteInput_View);
        break;
    case 1:
        byte_input_set_header_text(app->byteInput, "Set Function");
        byte_input_set_result_callback(app->byteInput, SetValue, NULL, app, &FUNCTION, 1);
        view_dispatcher_switch_to_view(app->viewDispatcher, ByteInput_View);
        break;
    case 2:
        byte_input_set_header_text(app->byteInput, "Set Address");
        byte_input_set_result_callback(app->byteInput, SetValue, NULL, app, &buf[2], 2);
        view_dispatcher_switch_to_view(app->viewDispatcher, ByteInput_View);
        break;
    case 3:
        byte_input_set_header_text(app->byteInput, "Set value or quantity");
        byte_input_set_result_callback(app->byteInput, SetValue, NULL, app, &buf[4], 2);
        view_dispatcher_switch_to_view(app->viewDispatcher, ByteInput_View);
        break;
    default:
        if(FUNCTION == 0x0F)
            offset += 2;
        else
            offset += offset - 3;
        byte_input_set_header_text(app->byteInput, "Set x value");
        byte_input_set_result_callback(
            app->byteInput, SetValue, NULL, app, &buf[offset], FUNCTION == 0x0F ? 1 : 2);
        view_dispatcher_switch_to_view(app->viewDispatcher, ByteInput_View);
        break;
    }
}
bool app_scene_byte_input_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}
void app_scene_byte_input_on_exit(void* context) {
    App* app = context;
    UNUSED(app);
}
