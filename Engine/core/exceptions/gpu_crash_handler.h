#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <wrl/client.h>

#include "core/core.h"

#include "NsightAftermathGpuCrashTracker.h"

struct IDxcBlob;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct D3D12_SHADER_BYTECODE;

namespace engine
{
  // Wrapper for D3D12HelloNsightAftermath GpuCrashTracker class
  struct fgpu_crash_tracker
  {
    CTOR_DEFAULT(fgpu_crash_tracker)
    CTOR_MOVE_COPY_DELETE(fgpu_crash_tracker)
    DTOR_DEFAULT(fgpu_crash_tracker)
    
    void pre_device_creation(uint32_t back_buffer_count);
    void post_device_creation(ID3D12Device* device);
    void create_context_handle(uint32_t back_buffer_index, ID3D12GraphicsCommandList* command_list);
    void wait_for_dump_and_throw(HRESULT hr);
    void advance_frame();

    uint64_t add_shader_binary(IDxcBlob* shader_blob);
    std::string add_source_shader_debug_data(IDxcBlob* shader_blob, IDxcBlob* pdb_blob);
    
  private:
    std::unique_ptr<GpuCrashTracker> impl;
  };
}
