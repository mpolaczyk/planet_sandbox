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

#define USE_CUSTOM_COM_POINTER 0
#define USE_NSIGHT_AFTERMATH 0
#define USE_NSIGHT_GRAPHICS 0
#define USE_FXC 0         // 0 means use DXC
#define FORCE_COMPILE_SHADERS_ON_START 1

#ifdef BUILD_DLL
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#define ALIGN(x) __declspec(align(x))

#define CTOR_DTOR(NAME) NAME(); ~NAME();
#define CTOR_VDTOR(NAME) NAME(); virtual ~NAME();
#define CTOR_DEFAULT(NAME) NAME() = default;
#define CTOR_DELETE(NAME) NAME() = delete;
#define DTOR_DEFAULT(NAME) ~NAME() = default;
#define VDTOR_DEFAULT(NAME) virtual ~NAME() = default;
#define DTOR_DELETE(NAME) ~NAME() = delete;
#define CTOR_MOVE_DEFAULT(NAME) \
  NAME(NAME&&) = default; \
  NAME& operator=(NAME&&) = default; 
#define CTOR_MOVE_DELETE(NAME) \
  NAME(NAME&&) = delete; \
  NAME& operator=(NAME&&) = delete;
#define CTOR_COPY_DEFAULT(NAME) \
  NAME(const NAME&) = default; \
  NAME& operator=(const NAME&) = default;
#define CTOR_COPY_DELETE(NAME) \
  NAME(const NAME&) = delete; \
  NAME& operator=(const NAME&) = delete;
#define CTOR_MOVE_COPY_DEFAULT(NAME) \
    CTOR_MOVE_DEFAULT(NAME) \
    CTOR_COPY_DEFAULT(NAME)
#define CTOR_MOVE_COPY_DELETE(NAME) \
  CTOR_MOVE_DELETE(NAME) \
  CTOR_COPY_DELETE(NAME)

#define RAND_SEED_FUNC(seed) rand_pcg(seed)

#define SPDLOG_USE_STD_FORMAT
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#define ALIGNED_STRUCT_BEGIN(NAME) struct alignas(16) NAME
#define ALIGNED_STRUCT_END(NAME) static_assert(sizeof(NAME) % 16 == 0);

#define MAX_MATERIALS 32
#define MAX_LIGHTS 16
#define MAX_TEXTURES 32
#define MAX_MAIN_DESCRIPTORS 128
#define MAX_RTV_DESCRIPTORS 16
#define MAX_DSV_DESCRIPTORS 4

// warning C4251 : ? needs to have dll - interface to be used by clients of struct ? (compiling source file ?)
// for standard library types!
#pragma warning( disable : 4251)

// warning C4275: non dll-interface class ? used as base for dll-interface class ? (compiling source file ?)
#pragma warning( disable : 4275)

extern int g_frame_number;
extern float g_frame_time_ms;
