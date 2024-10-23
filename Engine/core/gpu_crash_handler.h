#pragma once

#include <vector>
#include <wrl/client.h>

#include "d3d12.h"
#include "NsightAftermathGpuCrashTracker.h"

namespace engine
{
  using Microsoft::WRL::ComPtr;

  // Class based on D3D12HelloNsightAftermath sample project
  struct fgpu_crash_handler
  {
    fgpu_crash_handler();
    
    void pre_device_initialize(int back_buffer_count);
    void post_device_initialize(ComPtr<ID3D12Device> device);
    void create_context_handle(int back_buffer_index, ComPtr<ID3D12GraphicsCommandList> command_list);
    void wait_for_crash_and_throw(HRESULT hr);
    void advance_markers_frame();
    void set_marker(int back_buffer_index, const std::string& markerData, bool appManagedMarker);
    
    // App-managed marker functionality
    uint64_t frame_counter;
    MarkerMap marker_map;

    // Nsight Aftermath instrumentation, index - back buffer id
    std::vector<GFSDK_Aftermath_ContextHandle> aftermath_command_list_context;
    GpuCrashTracker crash_tracker;
  };
}
