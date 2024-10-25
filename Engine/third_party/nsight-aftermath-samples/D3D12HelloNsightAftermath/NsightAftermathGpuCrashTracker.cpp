//*********************************************************
//
// Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//*********************************************************

#include <string>
#include <fstream>

#include "NsightAftermathGpuCrashTracker.h"

#include "dxcapi.h"
#include "engine/log.h" // WATCH OUT! dependency on the project, but I want logging to be consistent

GpuCrashTracker::~GpuCrashTracker()
{
    if (m_initialized)
    {
        GFSDK_Aftermath_DisableGpuCrashDumps();
    }
}

void GpuCrashTracker::PreDeviceInitialize(int backBufferCount, const char* descriptionAppName)
{
    m_appName = descriptionAppName;
    for(int i = 0; i < backBufferCount; i++)
    {
      m_CommandListContext.push_back(nullptr);
    }
  
    // Enable GPU crash dumps and set up the callbacks for crash dump notifications,
    // shader debug information notifications, and providing additional crash
    // dump description data.Only the crash dump callback is mandatory. The other two
    // callbacks are optional and can be omitted, by passing nullptr, if the corresponding
    // functionality is not used.
    // The DeferDebugInfoCallbacks flag enables caching of shader debug information data
    // in memory. If the flag is set, ShaderDebugInfoCallback will be called only
    // in the event of a crash, right before GpuCrashDumpCallback. If the flag is not set,
    // ShaderDebugInfoCallback will be called for every shader that is compiled.
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_EnableGpuCrashDumps(
        GFSDK_Aftermath_Version_API,
        GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_DX,
        GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks, // Let the Nsight Aftermath library cache shader debug information.
        CrashDumpCallback,
        ShaderDebugInfoCallback,
        CrashDumpDescriptionCallback,
        ResolveMarkerCallback,
        this));
}

void GpuCrashTracker::PostDeviceInitialize(ID3D12Device* const device)
{
    const uint32_t flags =
      GFSDK_Aftermath_FeatureFlags_EnableMarkers |
      GFSDK_Aftermath_FeatureFlags_EnableResourceTracking |
      GFSDK_Aftermath_FeatureFlags_CallStackCapturing |
      GFSDK_Aftermath_FeatureFlags_GenerateShaderDebugInfo |
      GFSDK_Aftermath_FeatureFlags_EnableShaderErrorReporting;
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_Initialize(GFSDK_Aftermath_Version_API, flags, device));
    m_initialized = true;
}

void GpuCrashTracker::CreateContextHandle(int backBufferIndex, ID3D12GraphicsCommandList* commandList)
{
  AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_CreateContextHandle(commandList, &m_CommandListContext[backBufferIndex]));
}

void GpuCrashTracker::WaitForDump(std::string& outMsg)
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
  
  while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed && status != GFSDK_Aftermath_CrashDump_Status_Finished
    && tElapsed < tdrTerminationTimeout)
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
  outMsg = err_msg.str();
}

uint64_t GpuCrashTracker::AddShaderBinary(IDxcBlob* shaderBlob)
{
  const D3D12_SHADER_BYTECODE shader{ shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize() };

  GFSDK_Aftermath_ShaderBinaryHash hash;
  AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetShaderHash(GFSDK_Aftermath_Version_API, &shader, &hash));

  uint8_t* buff = static_cast<uint8_t*>(const_cast<void*>(shader.pShaderBytecode));
  size_t length = shader.BytecodeLength;
  std::vector<uint8_t> data(buff, buff + length);
  m_shaderBinaries[hash].swap(data);
  return hash.hash;
}

std::string GpuCrashTracker::AddSourceShaderDebugData(IDxcBlob* shaderBlob, IDxcBlob* pdbBlob)
{
  const D3D12_SHADER_BYTECODE shader{ shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize() };

  GFSDK_Aftermath_ShaderDebugName debug_name;
  AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetShaderDebugName(GFSDK_Aftermath_Version_API, &shader, &debug_name));

  uint8_t* buff = static_cast<uint8_t*>(const_cast<void*>(pdbBlob->GetBufferPointer()));
  size_t length = pdbBlob->GetBufferSize();
  std::vector<uint8_t> data(buff, buff + length);
  m_sourceShaderDebugData[debug_name].swap(data);
  return debug_name.name;
}

bool GpuCrashTracker::FindShaderBinary(const GFSDK_Aftermath_ShaderBinaryHash& shaderHash, std::vector<uint8_t>& shader) const
{
  // Find shader binary data for the shader hash
  auto i_shader = m_shaderBinaries.find(shaderHash);
  if (i_shader == m_shaderBinaries.end())
  {
    return false;
  }

  shader = i_shader->second;
  return true;
}

bool GpuCrashTracker::FindSourceShaderDebugData(const GFSDK_Aftermath_ShaderDebugName& shaderDebugName, std::vector<uint8_t>& debugData) const
{
  // Find shader debug data for the shader debug name.
  auto i_data = m_sourceShaderDebugData.find(shaderDebugName);
  if (i_data == m_sourceShaderDebugData.end())
  {
    return false;
  }

  debugData = i_data->second;
  return true;
}

void GpuCrashTracker::AdvanceFrame()
{
    m_frameCounter++;
    m_markerMap[m_frameCounter % c_markerFrameHistory].clear();
}

void GpuCrashTracker::SetMarker(int backBufferIndex, const std::string& markerData, bool appManagedMarker)
{
    if (appManagedMarker)
    {
        // App is responsible for handling marker memory, and for resolving the memory at crash dump generation time.
        // The actual "const void* markerData" passed to Aftermath in this case can be any uniquely identifying value that the app can resolve to the marker data later.
        // For this sample, we will use this approach to generating a unique marker value:
        // We keep a ringbuffer with a marker history of the last c_markerFrameHistory frames (currently 4).
        UINT markerMapIndex = m_frameCounter % c_markerFrameHistory;
        auto& currentFrameMarkerMap = m_markerMap[markerMapIndex];
        // Take the index into the ringbuffer, multiply by 10000, and add the total number of markers logged so far in the current frame, +1 to avoid a value of zero.
        size_t markerID = markerMapIndex * 10000 + currentFrameMarkerMap.size() + 1;
        // This value is the unique identifier we will pass to Aftermath and internally associate with the marker data in the map.
        currentFrameMarkerMap[markerID] = markerData;
        AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(m_CommandListContext[backBufferIndex], (void*)markerID, 0));
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
        AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(m_CommandListContext[backBufferIndex], (void*)markerData.c_str(), (unsigned int)markerData.size() + 1));
    }
}


void GpuCrashTracker::CrashDumpCallback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData)
{
    GpuCrashTracker* pGpuCrashTracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
    pGpuCrashTracker->OnCrashDump(pGpuCrashDump, gpuCrashDumpSize);
}
void GpuCrashTracker::OnCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize)
{
    // Make sure only one thread at a time...
    std::lock_guard<std::mutex> lock(m_mutex);

    // Write to file for later in-depth analysis with Nsight Graphics.
    WriteGpuCrashDumpToFile(pGpuCrashDump, gpuCrashDumpSize);
}

void GpuCrashTracker::ShaderDebugInfoCallback(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData)
{
    GpuCrashTracker* pGpuCrashTracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
    pGpuCrashTracker->OnShaderDebugInfo(pShaderDebugInfo, shaderDebugInfoSize);
}
void GpuCrashTracker::OnShaderDebugInfo(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize)
{
    // Make sure only one thread at a time...
    std::lock_guard<std::mutex> lock(m_mutex);

    // Get shader debug information identifier
    GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier = {};
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetShaderDebugInfoIdentifier(
        GFSDK_Aftermath_Version_API,
        pShaderDebugInfo,
        shaderDebugInfoSize,
        &identifier));

    // Store information for decoding of GPU crash dumps with shader address mapping
    // from within the application.
    std::vector<uint8_t> data((uint8_t*)pShaderDebugInfo, (uint8_t*)pShaderDebugInfo + shaderDebugInfoSize);
    m_shaderDebugInfo[identifier].swap(data);

    // Write to file for later in-depth analysis of crash dumps with Nsight Graphics
    WriteShaderDebugInformationToFile(identifier, pShaderDebugInfo, shaderDebugInfoSize);
}

void GpuCrashTracker::CrashDumpDescriptionCallback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void* pUserData)
{
    GpuCrashTracker* pGpuCrashTracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
    pGpuCrashTracker->OnCrashDumpDescription(addDescription);
}
void GpuCrashTracker::OnCrashDumpDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription)
{
    // Add some basic description about the crash. This is called after the GPU crash happens, but before
    // the actual GPU crash dump callback. The provided data is included in the crash dump and can be
    // retrieved using GFSDK_Aftermath_GpuCrashDump_GetDescription().
    addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, m_appName);
}

void GpuCrashTracker::ResolveMarkerCallback(const void* pMarkerData, const uint32_t markerDataSize, void* pUserData, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize)
{
    GpuCrashTracker* pGpuCrashTracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
    pGpuCrashTracker->OnResolveMarker(pMarkerData, markerDataSize, ppResolvedMarkerData, pResolvedMarkerDataSize);
}
void GpuCrashTracker::OnResolveMarker(const void* pMarkerData, const uint32_t markerDataSize, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize)
{
    // Important: the pointer passed back via ppResolvedMarkerData must remain valid after this function returns
    // using references for all of the m_markerMap accesses ensures that the pointers refer to the persistent data
    for (auto& map : m_markerMap)
    {
        auto foundMarker = map.find((uint64_t)pMarkerData);
        if (foundMarker != map.end())
        {
            const std::string& foundMarkerData = foundMarker->second;
            // std::string::data() will return a valid pointer until the string is next modified
            // we don't modify the string after calling data() here, so the pointer should remain valid
            *ppResolvedMarkerData = (void*)foundMarkerData.data();
            *pResolvedMarkerDataSize = (uint32_t)foundMarkerData.length();
            return;
        }
    }
}



void GpuCrashTracker::ShaderDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo, void* pUserData)
{
    GpuCrashTracker* pGpuCrashTracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
    pGpuCrashTracker->OnShaderDebugInfoLookup(*pIdentifier, setShaderDebugInfo);
}
void GpuCrashTracker::OnShaderDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo) const
{
    // Handler for shader debug information lookup callbacks.
    // This is used by the JSON decoder for mapping shader instruction
    // addresses to DXIL lines or HLSl source lines.
  
    // Search the list of shader debug information blobs received earlier.
    auto i_debugInfo = m_shaderDebugInfo.find(identifier);
    if (i_debugInfo == m_shaderDebugInfo.end())
    {
        // Early exit, nothing found. No need to call setShaderDebugInfo.
        LOG_ERROR("Unable to find shader debug info for identifier {0} {1}", identifier.id[0], identifier.id[1])
        return;
    }

    // Let the GPU crash dump decoder know about the shader debug information
    // that was found.
    setShaderDebugInfo(i_debugInfo->second.data(), uint32_t(i_debugInfo->second.size()));
}

void GpuCrashTracker::ShaderLookupCallback(const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData)
{
    GpuCrashTracker* pGpuCrashTracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
    pGpuCrashTracker->OnShaderLookup(*pShaderHash, setShaderBinary);
}
void GpuCrashTracker::OnShaderLookup(const GFSDK_Aftermath_ShaderBinaryHash& shaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary) const
{
    // Handler for shader lookup callbacks.
    // This is used by the JSON decoder for mapping shader instruction
    // addresses to DXIL lines or HLSL source lines.
    // NOTE: If the application loads stripped shader binaries (-Qstrip_debug),
    // Aftermath will require access to both the stripped and the not stripped
    // shader binaries.
  
    // Find shader binary data for the shader hash in the shader database.
    std::vector<uint8_t> shaderBinary;
    if (!FindShaderBinary(shaderHash, shaderBinary))
    {
        // Early exit, nothing found. No need to call setShaderBinary.
        LOG_ERROR("Unable to find shader binary for hash {0}", shaderHash.hash)
        return;
    }

    // Let the GPU crash dump decoder know about the shader data
    // that was found.
    setShaderBinary(shaderBinary.data(), uint32_t(shaderBinary.size()));
}

void GpuCrashTracker::ShaderSourceDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData)
{
    GpuCrashTracker* pGpuCrashTracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
    pGpuCrashTracker->OnShaderSourceDebugInfoLookup(*pShaderDebugName, setShaderBinary);
}
void GpuCrashTracker::OnShaderSourceDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugName& shaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary) const
{
    // Handler for shader source debug info lookup callbacks.
    // This is used by the JSON decoder for mapping shader instruction addresses to
    // HLSL source lines, if the shaders used by the application were compiled with
    // separate debug info data files.
  
    // Find source debug info for the shader DebugName in the shader database.
    std::vector<uint8_t> sourceDebugInfo;
    if (!FindSourceShaderDebugData(shaderDebugName, sourceDebugInfo))
    {
        // Early exit, nothing found. No need to call setShaderBinary.
        LOG_ERROR("Unable to find shader source debug info for debug name {0}", shaderDebugName.name)
        return;
    }

    // Let the GPU crash dump decoder know about the shader debug data that was
    // found.
    setShaderBinary(sourceDebugInfo.data(), uint32_t(sourceDebugInfo.size()));
}



void GpuCrashTracker::WriteGpuCrashDumpToFile(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize)
{
    // Create a GPU crash dump decoder object for the GPU crash dump.
    GFSDK_Aftermath_GpuCrashDump_Decoder decoder = {};
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_CreateDecoder(
        GFSDK_Aftermath_Version_API,
        pGpuCrashDump,
        gpuCrashDumpSize,
        &decoder));

    // Use the decoder object to read basic information, like application
    // name, PID, etc. from the GPU crash dump.
    GFSDK_Aftermath_GpuCrashDump_BaseInfo baseInfo = {};
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetBaseInfo(decoder, &baseInfo));

    // Use the decoder object to query the application name that was set
    // in the GPU crash dump description.
    uint32_t applicationNameLength = 0;
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize(
        decoder,
        GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
        &applicationNameLength));

    std::vector<char> applicationName(applicationNameLength, '\0');

    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetDescription(
        decoder,
        GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
        uint32_t(applicationName.size()),
        applicationName.data()));

    // Create a unique file name for writing the crash dump data to a file.
    // Note: due to an Nsight Aftermath bug (will be fixed in an upcoming
    // driver release) we may see redundant crash dumps. As a workaround,
    // attach a unique count to each generated file name.
    static int count = 0;
    const std::string baseFileName =
        std::string(applicationName.data())
        + "-"
        + std::to_string(baseInfo.pid)
        + "-"
        + std::to_string(++count);

    // Write the crash dump data to a file using the .nv-gpudmp extension
    // registered with Nsight Graphics.
    const std::string crashDumpFileName = baseFileName + ".nv-gpudmp";
    std::ofstream dumpFile(crashDumpFileName, std::ios::out | std::ios::binary);
    if (dumpFile)
    {
        dumpFile.write((const char*)pGpuCrashDump, gpuCrashDumpSize);
        dumpFile.close();
    }

    // Decode the crash dump to a JSON string.
    // Step 1: Generate the JSON and get the size.
    uint32_t jsonSize = 0;
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GenerateJSON(
        decoder,
        GFSDK_Aftermath_GpuCrashDumpDecoderFlags_ALL_INFO,
        GFSDK_Aftermath_GpuCrashDumpFormatterFlags_NONE,
        ShaderDebugInfoLookupCallback,
        ShaderLookupCallback,
        ShaderSourceDebugInfoLookupCallback,
        this,
        &jsonSize));
    // Step 2: Allocate a buffer and fetch the generated JSON.
    std::vector<char> json(jsonSize);
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetJSON(
        decoder,
        uint32_t(json.size()),
        json.data()));

    // Write the crash dump data as JSON to a file.
    const std::string jsonFileName = crashDumpFileName + ".json";
    std::ofstream jsonFile(jsonFileName, std::ios::out | std::ios::binary);
    if (jsonFile)
    {
       // Write the JSON to the file (excluding string termination)
       jsonFile.write(json.data(), json.size() - 1);
       jsonFile.close();
    }

    // Destroy the GPU crash dump decoder object.
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_DestroyDecoder(decoder));
}

void GpuCrashTracker::WriteShaderDebugInformationToFile(GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier, const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize)
{
    // Create a unique file name.
    const std::string filePath = "shader-" + std::to_string(identifier) + ".nvdbg";

    std::ofstream f(filePath, std::ios::out | std::ios::binary);
    if (f)
    {
        f.write((const char*)pShaderDebugInfo, shaderDebugInfoSize);
    }
}







