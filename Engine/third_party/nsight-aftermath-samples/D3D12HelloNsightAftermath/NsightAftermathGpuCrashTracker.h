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

#pragma once

#include <map>
#include <array>
#include <vector>
#include <mutex>

#include "d3d12.h"  // Required mostly for __d3d12_h__ define, rest can be forward declared
#include "GFSDK_Aftermath.h"
#include "GFSDK_Aftermath_GpuCrashDump.h"
#include "NsightAftermathHelpers.h"

struct IDxcBlob;

// Implements GPU crash dump tracking using the Nsight Aftermath API.
// This class is heavliy modified. All sample functionality is merged here to avoid it's terrible import structure. 
class GpuCrashTracker
{
public:
    GpuCrashTracker() = default;
    ~GpuCrashTracker();
  
    void PreDeviceInitialize(int backBufferCount, const char* descriptionAppName);
    void PostDeviceInitialize(ID3D12Device* const device);
  
    void CreateContextHandle(uint32_t backBufferIndex, ID3D12GraphicsCommandList* commandList);
  
    void WaitForDump(std::string& outMsg);

    uint64_t AddShaderBinary(IDxcBlob* shaderBlob);
    std::string AddSourceShaderDebugData(IDxcBlob* shaderBlob, IDxcBlob* pdbBlob);
  
    void AdvanceFrame();
    void SetMarker(int backBufferIndex, const std::string& markerData, bool appManagedMarker);
  
private:
    bool m_initialized = false;
    mutable std::mutex m_mutex;
    const char* m_appName = nullptr;
  
    // Markers functionality
    static constexpr uint8_t c_markerFrameHistory = 4;
    uint64_t m_frameCounter = 0;
    std::array<std::map<uint64_t, std::string>, c_markerFrameHistory> m_markerMap;
    std::vector<GFSDK_Aftermath_ContextHandle> m_CommandListContext; // Index - back buffer id

    // Shader database functionality
    std::map<GFSDK_Aftermath_ShaderDebugInfoIdentifier, std::vector<uint8_t>> m_shaderDebugInfo;  // Filled automatically in OnShaderDebugInfo
    std::map<GFSDK_Aftermath_ShaderBinaryHash, std::vector<uint8_t>> m_shaderBinaries;            // Registered by hand in AddShaderBinary
    std::map<GFSDK_Aftermath_ShaderDebugName, std::vector<uint8_t>> m_sourceShaderDebugData;      // Registered by hand in AddSourceShaderDebugData
  
    // GFSDK_Aftermath_EnableGpuCrashDumps callbacks
    static void CrashDumpCallback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData);
    static void ShaderDebugInfoCallback(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData);
    static void CrashDumpDescriptionCallback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void* pUserData);
    static void ResolveMarkerCallback(const void* pMarkerData, const uint32_t markerDataSize, void* pUserData, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize);
    void OnCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);
    void OnShaderDebugInfo(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize);
    void OnCrashDumpDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription);
    void OnResolveMarker(const void* pMarkerData, const uint32_t markerDataSize, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize);

    // GFSDK_Aftermath_GpuCrashDump_GenerateJSON callbacks
    static void ShaderDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo, void* pUserData);
    static void ShaderLookupCallback(const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData);
    static void ShaderSourceDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData);
    void OnShaderDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo) const;
    void OnShaderLookup(const GFSDK_Aftermath_ShaderBinaryHash& shaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary) const;
    void OnShaderSourceDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugName& shaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary) const;

    // Other helpers
    void WriteGpuCrashDumpToFile(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);
    void WriteShaderDebugInformationToFile(GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier, const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize);
    bool FindShaderBinary(const GFSDK_Aftermath_ShaderBinaryHash& shaderHash, std::vector<uint8_t>& shader) const;
    bool FindSourceShaderDebugData(const GFSDK_Aftermath_ShaderDebugName& shaderDebugName, std::vector<uint8_t>& debugData) const;
};
