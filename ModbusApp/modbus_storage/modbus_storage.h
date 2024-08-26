#pragma once

#include <furi.h>

#include "../Modbus.h"

char* sequential_file_resolve_path(
    Storage* storage,
    const char* dir,
    const char* prefix,
    const char* extension);
bool OpenLogFile(App* app);
void makePaths(App* app);
void open_log_file_stream(void* context);
void close_log_file_stream(void* context);
