#include "gpu_crash_handler.h"

#include "core/exceptions.h"
#include "engine/log.h"


#include "NsightAftermathHelpers.h"
#include "NsightAftermathGpuCrashTracker.h"

namespace engine
{
  fgpu_crash_handler::fgpu_crash_handler()
    : m_gpuCrashTracker(m_markerMap)
  {
  }

  void fgpu_crash_handler::pre_device_initialize(int back_buffer_count)
  {
    m_gpuCrashTracker.Initialize();
    for(int i = 0; i < back_buffer_count; i++)
    {
      m_hAftermathCommandListContext.push_back(nullptr);

    }
    LOG_WARN("Nsight Aftermath is enabled, disabling the DX12 debug layer!")
  }

  void fgpu_crash_handler::post_device_initialize(ComPtr<ID3D12Device> device)
  {
    const uint32_t flags =
      GFSDK_Aftermath_FeatureFlags_EnableMarkers |
      GFSDK_Aftermath_FeatureFlags_EnableResourceTracking |
      GFSDK_Aftermath_FeatureFlags_CallStackCapturing |
      GFSDK_Aftermath_FeatureFlags_GenerateShaderDebugInfo;
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_Initialize(GFSDK_Aftermath_Version_API, flags, device.Get()));
  }

  void fgpu_crash_handler::create_context_handle(int back_buffer_index, ComPtr<ID3D12GraphicsCommandList> command_list)
  {
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_CreateContextHandle(command_list.Get(), &m_hAftermathCommandListContext[back_buffer_index]));
  }

  void fgpu_crash_handler::wait_for_crash_and_throw(HRESULT hr)
  {
    using namespace std::chrono;
    
    // DXGI_ERROR error notification is asynchronous to the NVIDIA display
    // driver's GPU crash handling. Give the Nsight Aftermath GPU crash dump
    // thread some time to do its work before terminating the process.
    auto tdrTerminationTimeout = seconds(3);
    auto tStart = steady_clock::now();
    auto tElapsed = milliseconds::zero();

    GFSDK_Aftermath_CrashDump_Status status = GFSDK_Aftermath_CrashDump_Status_Unknown;
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

    while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed &&
           status != GFSDK_Aftermath_CrashDump_Status_Finished &&
           tElapsed < tdrTerminationTimeout)
    {
      // Sleep 50ms and poll the status again until timeout or Aftermath finished processing the crash dump.
      std::this_thread::sleep_for(milliseconds(50));
      AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

      auto tEnd = steady_clock::now();
      tElapsed = duration_cast<milliseconds>(tEnd - tStart);
    }

    std::stringstream err_msg;
    if (status == GFSDK_Aftermath_CrashDump_Status_Finished)
    {
      err_msg << "GPU crashed. Search for the crash dump.";
    }
    else
    {
      err_msg << "GPU crashed. Unexpected crash dump status: " << status << " timeout after: " << duration_cast<seconds>(tElapsed).count() << " seconds.";
    }
    throw fhresult_exception(hr, err_msg.str());
  }

  void fgpu_crash_handler::advance_markers_frame()
  {
    m_frameCounter++;
  }


}