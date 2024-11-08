#pragma once

#include <cstdint>
#include <string>
#include <wrl/client.h>

struct IDxcBlob;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct D3D12_SHADER_BYTECODE;
class GpuCrashTracker;

namespace engine
{
  // Wrapper for D3D12HelloNsightAftermath GpuCrashTracker class
  struct fgpu_crash_tracker
  {
    fgpu_crash_tracker();
    ~fgpu_crash_tracker();
    fgpu_crash_tracker(const fgpu_crash_tracker&) = delete;
    fgpu_crash_tracker(fgpu_crash_tracker&&) = delete;
    fgpu_crash_tracker& operator=(const fgpu_crash_tracker&) = delete;
    fgpu_crash_tracker& operator=(fgpu_crash_tracker&&) = delete;
    
    void pre_device_initialize(uint32_t back_buffer_count);
    void post_device_initialize(ID3D12Device* device);
    void create_context_handle(uint32_t back_buffer_index, ID3D12GraphicsCommandList* command_list);
    void wait_for_dump_and_throw(HRESULT hr);
    void advance_frame();

    uint64_t add_shader_binary(IDxcBlob* shader_blob);
    std::string add_source_shader_debug_data(IDxcBlob* shader_blob, IDxcBlob* pdb_blob);
    
  private:
    GpuCrashTracker* impl = nullptr;
  };
}
