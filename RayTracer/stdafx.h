#pragma once

// Common library headers
#include <string>
#include <vector>

#include <ppl.h>

// Build type detection
#ifdef _DEBUG
#define BUILD_DEBUG 1
#define BUILD_RELEASE 0
#elif NDEBUG
#define BUILD_DEBUG 0
#define BUILD_RELEASE 1
#endif

// Defines per build configuration
#if BUILD_DEBUG
#define USE_BENCHMARK 1   // Use time measurement in benchmark namespace
#define USE_PIX 1         // Use PIX events in benchmark namespace
#define USE_SIMD 1        // Use hand crafted SIMD code in math functions
#define USE_FPEXCEPT 1    // Use floating point exceptions. Remember to set /fp:except and /EHa in the compiler setting
#define USE_STAT 1        // Use atomic counters in stats namespace
#define USE_TLAS 1        // Use top level acceleration structure for scene objects
#elif BUILD_RELEASE
#define USE_BENCHMARK 1
#define USE_PIX 0
#define USE_SIMD 1
#define USE_FPEXCEPT 0
#define USE_STAT 1
#define USE_TLAS 1
#endif

// Third party defines
#define TINYOBJLOADER_IMPLEMENTATION

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

#define SPDLOG_USE_STD_FORMAT
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

// Common project headers
#include "math/common.h"

// Declare all globals here
class asset_registry;
namespace globals
{
  asset_registry* get_asset_registry();
}