#include "../Modbus.h"
#include "../modbus_storage/modbus_storage.h"

//////////////////////////   Main Scene  //////////////////////////
void mainOptionsCB(void* context, uint32_t index) {
    App* app = context;

    switch(index) {
    case Settings_Option:
        scene_manager_set_scene_state(app->sceneManager, app_scene_main, Settings_Option);
        scene_manager_next_scene(app->sceneManager, app_scene_settings);
        break;
    case Sniffer_Option:
        scene_manager_set_scene_state(app->sceneManager, app_scene_main, Sniffer_Option);
        scene_manager_set_scene_state(app->sceneManager, app_scene_sniffer, Sniffer_Option);
        scene_manager_next_scene(app->sceneManager, app_scene_sniffer);
        break;
    case Sender_Option:
        scene_manager_set_scene_state(app->sceneManager, app_scene_main, Sender_Option);
        scene_manager_next_scene(app->sceneManager, app_scene_sender);
        break;
    case Read_LOG_Option:
        scene_manager_set_scene_state(app->sceneManager, app_scene_main, Read_LOG_Option);
        close_log_file_stream(app);
        if(OpenLogFile(app)) {
            scene_manager_set_scene_state(app->sceneManager, app_scene_sniffer, Read_LOG_Option);
            scene_manager_next_scene(app->sceneManager, app_scene_sniffer);
        }
        break;
    case About_Option:
        scene_manager_set_scene_state(app->sceneManager, app_scene_main, About_Option);
        scene_manager_set_scene_state(app->sceneManager, app_scene_sniffer, About_Option);
        scene_manager_next_scene(app->sceneManager, app_scene_sniffer);
        break;
    default:
        break;
    }
}

void app_scene_main_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->subMenu);
    submenu_set_header(app->subMenu, "Main");
    submenu_add_item(app->subMenu, "Settings", Settings_Option, mainOptionsCB, app);
    submenu_add_item(app->subMenu, "Sniffer", Sniffer_Option, mainOptionsCB, app);
    submenu_add_item(app->subMenu, "Sender", Sender_Option, mainOptionsCB, app);
    submenu_add_item(app->subMenu, "Read LOG", Read_LOG_Option, mainOptionsCB, app);
    submenu_add_item(app->subMenu, "About", About_Option, mainOptionsCB, app);
    submenu_set_selected_item(
        app->subMenu, scene_manager_get_scene_state(app->sceneManager, app_scene_main));
    view_dispatcher_switch_to_view(app->viewDispatcher, Submenu_View);
}
bool app_scene_main_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}
void app_scene_main_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->subMenu);
}
