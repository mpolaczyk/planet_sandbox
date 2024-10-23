#include "gpu_crash_handler.h"

#include "core/exceptions.h"
#include "engine/log.h"


#include "NsightAftermathHelpers.h"
#include "NsightAftermathGpuCrashTracker.h"
#include "engine/io.h"

namespace engine
{
  fgpu_crash_handler::fgpu_crash_handler()
    : crash_tracker(marker_map)
  {
  }

  void fgpu_crash_handler::pre_device_initialize(int back_buffer_count)
  {
    crash_tracker.Initialize(fio::get_shaders_dir().c_str());
    for(int i = 0; i < back_buffer_count; i++)
    {
      aftermath_command_list_context.push_back(nullptr);
    }
    LOG_INFO("Nsight Aftermath is enabled, disabling the DX12 debug layer!")
  }

  void fgpu_crash_handler::post_device_initialize(ComPtr<ID3D12Device> device)
  {
    const uint32_t flags =
      GFSDK_Aftermath_FeatureFlags_EnableMarkers |
      GFSDK_Aftermath_FeatureFlags_EnableResourceTracking |
      GFSDK_Aftermath_FeatureFlags_CallStackCapturing |
      GFSDK_Aftermath_FeatureFlags_GenerateShaderDebugInfo |
      GFSDK_Aftermath_FeatureFlags_EnableShaderErrorReporting;
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_Initialize(GFSDK_Aftermath_Version_API, flags, device.Get()));
  }

  void fgpu_crash_handler::create_context_handle(int back_buffer_index, ComPtr<ID3D12GraphicsCommandList> command_list)
  {
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_CreateContextHandle(command_list.Get(), &aftermath_command_list_context[back_buffer_index]));
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
    frame_counter++;
    marker_map[frame_counter % c_markerFrameHistory].clear();
  }

  void fgpu_crash_handler::set_marker(int back_buffer_index, const std::string& markerData, bool appManagedMarker)
  {
    if (appManagedMarker)
    {
        // App is responsible for handling marker memory, and for resolving the memory at crash dump generation time.
        // The actual "const void* markerData" passed to Aftermath in this case can be any uniquely identifying value that the app can resolve to the marker data later.
        // For this sample, we will use this approach to generating a unique marker value:
        // We keep a ringbuffer with a marker history of the last c_markerFrameHistory frames (currently 4).
        UINT markerMapIndex = frame_counter % c_markerFrameHistory;
        auto& currentFrameMarkerMap = marker_map[markerMapIndex];
        // Take the index into the ringbuffer, multiply by 10000, and add the total number of markers logged so far in the current frame, +1 to avoid a value of zero.
        size_t markerID = markerMapIndex * 10000 + currentFrameMarkerMap.size() + 1;
        // This value is the unique identifier we will pass to Aftermath and internally associate with the marker data in the map.
        currentFrameMarkerMap[markerID] = markerData;
        AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(aftermath_command_list_context[back_buffer_index], (void*)markerID, 0));
        // For example, if we are on frame 625, markerMapIndex = 625 % 4 = 1...
        // The first marker for the frame will have markerID = 1 * 10000 + 0 + 1 = 10001.
        // The 15th marker for the frame will have markerID = 1 * 10000 + 14 + 1 = 10015.
        // On the next frame, 626, markerMapIndex = 626 % 4 = 2.
        // The first marker for this frame will have markerID = 2 * 10000 + 0 + 1 = 20001.
        // The 15th marker for the frame will have markerID = 2 * 10000 + 14 + 1 = 20015.
        // So with this scheme, we can safely have up to 10000 markers per frame, and can guarantee a unique markerID for each one.
        // There are many ways to generate and track markers and unique marker identifiers!
    }
    else
    {
        AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(aftermath_command_list_context[back_buffer_index], (void*)markerData.c_str(), (unsigned int)markerData.size() + 1));
    }
  }
}