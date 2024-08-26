#include "modbus_storage.h"

char* sequential_file_resolve_path(
    Storage* storage,
    const char* dir,
    const char* prefix,
    const char* extension) {
    if(storage == NULL || dir == NULL || prefix == NULL || extension == NULL) {
        return NULL;
    }

    char file_path[256];
    int file_index = 0;

    do {
        if(snprintf(
               file_path, sizeof(file_path), "%s/%s_%d.%s", dir, prefix, file_index, extension) <
           0) {
            return NULL;
        }
        file_index++;
    } while(storage_file_exists(storage, file_path));

    return strdup(file_path);
}

void makePaths(App* app) {
    furi_assert(app);
    if(!storage_simply_mkdir(app->storage, PATHAPPEXT)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot create\napp folder");
    }
    if(!storage_simply_mkdir(app->storage, PATHLOGS)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot create\nlogs folder");
    }
}
void open_log_file_stream(void* context) {
    App* app = context;
    strcpy(app->logFilePath, sequential_file_resolve_path(app->storage, PATHLOGS, "Log", "log"));
    if(app->logFilePath != NULL) {
        if(storage_file_open(app->LOGfile, app->logFilePath, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            furi_string_reset(app->text);
            app->LOGfileReady = true;
        } else {
            dialog_message_show_storage_error(app->dialogs, "Cannot open log file");
        }
    } else {
        dialog_message_show_storage_error(app->dialogs, "Cannot resolve log path");
    }
}
void close_log_file_stream(void* context) {
    App* app = context;
    if(app->LOGfile && storage_file_is_open(app->LOGfile)) {
        app->LOGfileReady = false;
        storage_file_close(app->LOGfile);
    }
}
bool OpenLogFile(App* app) {
    // browse for files
    FuriString* predefined_filepath = furi_string_alloc_set_str(PATHLOGS);
    FuriString* selected_filepath = furi_string_alloc();
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, ".log", NULL);
    if(!dialog_file_browser_show(
           app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
        return false;
    }
    if(storage_file_open(
           app->LOGfile, furi_string_get_cstr(selected_filepath), FSAM_READ, FSOM_OPEN_EXISTING)) {
        furi_string_reset(app->text);
        char buf[storage_file_size(app->LOGfile)];
        storage_file_read(app->LOGfile, buf, sizeof(buf));
        buf[sizeof(buf)] = '\0';
        furi_string_cat_str(app->text, buf);
    } else {
        dialog_message_show_storage_error(app->dialogs, "Cannot open File");
        return false;
    }
    storage_file_close(app->LOGfile);
    furi_string_free(selected_filepath);
    furi_string_free(predefined_filepath);
    return true;
}
