#pragma once

// Common library headers
#include <string>
#include <vector>
#include <cassert>

#include <ppl.h>
#include <sstream>

#include "engine.h"
using namespace engine;



#define IMGUI_DISABLE_DEMO_WINDOWS
#define IMGUI_DISABLE_METRICS_WINDOW

#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
#define IMGUI_ENABLE_WIN32_DEFAULT_IME_FUNCTIONS
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#define IMGUI_DISABLE_WIN32_FUNCTIONS
#define IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#define IMGUI_DISABLE_FILE_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS