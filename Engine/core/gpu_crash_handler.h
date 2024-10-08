#pragma once

#include <vector>
#include <wrl/client.h>



// #ifdef __d3d12_h__
// #include "GFSDK_Aftermath.h"
// #else
// // Hack for GFSDK_Aftermath.h, I don't want to include full d3d12.h here
// #define __d3d12_h__
// struct ID3D12Resource;
// struct ID3D12Device;
// #include "GFSDK_Aftermath.h"
// #undef __d3d12_h__
// #endif

// Code based on D3D12HelloNsightAftermath sample project

#include "d3d12.h"

//#include "GFSDK_Aftermath.h"
//#include "NsightAftermathCommon.h"
#include "NsightAftermathGpuCrashTracker.h"

//struct ID3D12Device;
//struct ID3D12GraphicsCommandList;
//
//class GpuCrashTracker;


namespace engine
{
  using Microsoft::WRL::ComPtr;
  
  struct fgpu_crash_handler
  {
    fgpu_crash_handler();
    
    void pre_device_initialize(int back_buffer_count);
    void post_device_initialize(ComPtr<ID3D12Device> device);
    void create_context_handle(int back_buffer_index, ComPtr<ID3D12GraphicsCommandList> command_list);
    void wait_for_crash_and_throw(HRESULT hr);
    void advance_markers_frame();
    
    // App-managed marker functionality
    uint64_t m_frameCounter;
    MarkerMap m_markerMap;

    // Nsight Aftermath instrumentation
    std::vector<GFSDK_Aftermath_ContextHandle> m_hAftermathCommandListContext;
    GpuCrashTracker m_gpuCrashTracker;
  };
}
