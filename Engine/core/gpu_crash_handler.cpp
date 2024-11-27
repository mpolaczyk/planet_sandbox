#include "gpu_crash_handler.h"

#include "core/exceptions.h"
#include "engine/log.h"

namespace engine
{
  void fgpu_crash_tracker::pre_device_creation(uint32_t back_buffer_count)
  {
    impl = std::make_unique<GpuCrashTracker>();
    impl->PreDeviceInitialize(back_buffer_count, "planetSandbox");
    LOG_DEBUG("Nsight Aftermath is enabled.")
  }

  void fgpu_crash_tracker::post_device_creation(ID3D12Device* device)
  {
    impl->PostDeviceInitialize(device);
  }

  void fgpu_crash_tracker::create_context_handle(uint32_t back_buffer_index, ID3D12GraphicsCommandList* command_list)
  {
    impl->CreateContextHandle(back_buffer_index, command_list);
  }

  void fgpu_crash_tracker::wait_for_dump_and_throw(HRESULT hr)
  {
    LOG_WARN("Waiting for the GPU crash dump!")
    std::string msg;
    impl->WaitForDump(msg);
    throw fhresult_exception(hr, std::move(msg));
  }

  void fgpu_crash_tracker::advance_frame()
  {
    impl->AdvanceFrame();
  }

  uint64_t fgpu_crash_tracker::add_shader_binary(IDxcBlob* shader_blob)
  {
    return impl->AddShaderBinary(shader_blob);
  }
  
  std::string fgpu_crash_tracker::add_source_shader_debug_data(IDxcBlob* shader_blob, IDxcBlob* pdb_blob)
  {
    return impl->AddSourceShaderDebugData(shader_blob, pdb_blob);
  }
}