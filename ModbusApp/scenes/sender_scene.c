#include "../Modbus.h"

//////////////////////////   Sender Scene  ////////////////////////
void SenderOptionsCB(void* context, uint32_t index) {
    App* app = context;
    if(index == Manual_Sender_Option) {
        scene_manager_set_scene_state(app->sceneManager, app_scene_sender, Manual_Sender_Option);
        scene_manager_next_scene(app->sceneManager, app_scene_manual_sender);
    } else if(index == Buffer_Sender_Option) {
        scene_manager_set_scene_state(app->sceneManager, app_scene_sender, Buffer_Sender_Option);
        scene_manager_next_scene(app->sceneManager, app_scene_msgs_buffer);
    }
}
void app_scene_sender_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->subMenu);
    submenu_set_header(app->subMenu, "Sender");
    submenu_add_item(app->subMenu, "Manual Sender", Manual_Sender_Option, SenderOptionsCB, app);
    submenu_add_item(app->subMenu, "Buffer Sender", Buffer_Sender_Option, SenderOptionsCB, app);
    submenu_set_selected_item(
        app->subMenu, scene_manager_get_scene_state(app->sceneManager, app_scene_main));
    view_dispatcher_switch_to_view(app->viewDispatcher, Submenu_View);
}
bool app_scene_sender_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}
void app_scene_sender_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->subMenu);
}
