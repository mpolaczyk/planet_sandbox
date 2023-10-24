#pragma once

#ifndef PLATFORM_WINDOWS
  #error Engine supports only Windows platform!
#endif


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
#define USE_STAT 0
#define USE_TLAS 1
#endif


#ifdef BUILD_DLL
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#define SPDLOG_USE_STD_FORMAT
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE