#include "stdafx.h"

#include "ray_tracing_pass.h"

#include "hittables/scene.h"
#include "hittables/static_mesh.h"

#include "assets/mesh.h"

#include "engine/window.h"
#include "engine/math/math.h"
#include "engine/string_tools.h"
#include "engine/renderer/aligned_structs.h"
#include "engine/renderer/command_list.h"
#include "engine/renderer/render_context.h"
#include "engine/renderer/scene_acceleration.h"
#include "engine/renderer/gpu_resources.h"
#include "engine/renderer/device.h"
#include "engine/renderer/pipeline.h"

namespace engine
{
  using namespace DirectX;

  namespace
  {    
    enum global_root_parameter_type : int
    {
      output_view = 0,
      acceleration_structure,
      scene_constant,
      vertex_buffer,
      num_global
    };

    enum local_root_parameter_type : int
    {
      cube_constant = 0,
      num_local
    };
  }

  void fray_tracing_pass::init_shaders()
  {
    ray_tracing_shader_asset.set_name("ray_tracing");
  }
  
  void fray_tracing_pass::init_pipeline()
  {
    fpass_base::init_pipeline();
    
    froot_signature global_sig;
    global_sig.reserve_parameters(global_root_parameter_type::num_global);
    global_sig.add_descriptor_table_parameter(global_root_parameter_type::output_view, 1, 0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
    global_sig.add_shader_resource_view_parameter(global_root_parameter_type::acceleration_structure, 0, 0);
    global_sig.add_constant_buffer_view_parameter(global_root_parameter_type::scene_constant, 0, 0);
    global_sig.add_descriptor_table_parameter(global_root_parameter_type::vertex_buffer, 1, 0, 2, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
    
    froot_signature local_sig;
    local_sig.reserve_parameters(local_root_parameter_type::num_local);
    local_sig.add_constant_parameter(local_root_parameter_type::cube_constant, 1, 0, sizeof(m_cubeCB));

    pipeline->root_signature_ray_tracing_global = global_sig;
    pipeline->root_signature_ray_tracing_local = local_sig;
    pipeline->init("Ray tracing pipeline");
  }

  inline void AllocateUploadBuffer(ID3D12Device* pDevice, void *pData, UINT64 datasize, ID3D12Resource **ppResource, const wchar_t* resourceName = nullptr)
  {
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
    THROW_IF_FAILED(pDevice->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(ppResource)));
    if (resourceName)
    {
      (*ppResource)->SetName(resourceName);
    }
    void *pMappedData;
    (*ppResource)->Map(0, nullptr, &pMappedData);
    memcpy(pMappedData, pData, datasize);
    (*ppResource)->Unmap(0, nullptr);
  }

  fdescriptor fray_tracing_pass::AllocateDescriptor()
  {
    fdescriptor desc;
    context->main_descriptor_heap->push(desc, "TODO find name");
    return desc;
  }

  inline void AllocateUAVBuffer(UINT64 bufferSize, ID3D12Resource **ppResource, D3D12_RESOURCE_STATES initialResourceState, const wchar_t* resourceName)
  {
    fdevice* device = fapplication::get_instance()->device.get();
    
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    THROW_IF_FAILED(device->com.Get()->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        initialResourceState,
        nullptr,
        IID_PPV_ARGS(ppResource)));
    if (resourceName)
    {
      (*ppResource)->SetName(resourceName);
    }
  }

  inline void AllocateUploadBuffer(void *pData, UINT64 datasize, ID3D12Resource **ppResource, const wchar_t* resourceName = nullptr)
  {
    fdevice* device = fapplication::get_instance()->device.get();
    
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
    THROW_IF_FAILED(device->com.Get()->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(ppResource)));
    if (resourceName)
    {
      (*ppResource)->SetName(resourceName);
    }
    void *pMappedData;
    (*ppResource)->Map(0, nullptr, &pMappedData);
    memcpy(pMappedData, pData, datasize);
    (*ppResource)->Unmap(0, nullptr);
  }
  
  // Create SRV for a buffer.
  void fray_tracing_pass::CreateBufferSRV(fshader_resource_buffer& buffer, UINT numElements, UINT elementSize)
  {
    fdevice* device = fapplication::get_instance()->device.get();

    // SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = numElements;
    if (elementSize == 0)
    {
      srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
      srvDesc.Buffer.StructureByteStride = 0;
    }
    else
    {
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
      srvDesc.Buffer.StructureByteStride = elementSize;
    }
    buffer.srv = AllocateDescriptor();
    device->com.Get()->CreateShaderResourceView(buffer.resource.Get(), &srvDesc, buffer.srv.cpu_descriptor_handle);
  }
  
  void fray_tracing_pass::init_size_independent_resources()
  {
    fdevice* device = fapplication::get_instance()->device.get();
    
    // ##### SCENE - const scene for now
    
    // Cube indices.
    uint16_t indices[] = { 3,1,0, 2,1,3, 6,4,5, 7,4,6, 11,9,8, 10,9,11, 14,12,13, 15,12,14, 19,17,16, 18,17,19, 22,20,21, 23,20,22 };

    // Cube vertices positions and corresponding triangle normals.
    Vertex vertices[] =
    {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) }, { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) }, { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) }, { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) }, { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) }, { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) }, { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) }, { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) }, { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) }, { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };

    AllocateUploadBuffer(device->com.Get(), indices, sizeof(indices), &m_indexBuffer.resource);
    AllocateUploadBuffer(device->com.Get(), vertices, sizeof(vertices), &m_vertexBuffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor table.
    // Vertex buffer descriptor must follow index buffer descriptor in the descriptor heap.
    CreateBufferSRV(m_indexBuffer, sizeof(indices)/4, 0);
    CreateBufferSRV(m_vertexBuffer, ARRAYSIZE(vertices), sizeof(vertices[0]));
    // TODO it's not a HR calue
    THROW_IF_FAILED(m_vertexBuffer.srv.index == m_indexBuffer.srv.index + 1);

   
  }
  
  void fray_tracing_pass::init_size_dependent_resources(bool cleanup)
  {
    // Nothing to do here
  }

  void fray_tracing_pass::draw(frenderer_context* in_context, fcommand_list* command_list)
  {
    fpass_base::draw(in_context, command_list);

    fdevice* device = fapplication::get_instance()->device.get();
    
    if(first_frame_done)
    {
      D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
      geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
      geometryDesc.Triangles.IndexBuffer = m_indexBuffer.resource->GetGPUVirtualAddress();
      geometryDesc.Triangles.IndexCount = static_cast<UINT>(m_indexBuffer.resource->GetDesc().Width) / sizeof(uint16_t);
      geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
      geometryDesc.Triangles.Transform3x4 = 0;
      geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
      geometryDesc.Triangles.VertexCount = static_cast<UINT>(m_vertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
      geometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer.resource->GetGPUVirtualAddress();
      geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
      
      // Mark the geometry as opaque. 
      // PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
      // Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
      geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
      
      // Get required sizes for an acceleration structure.
      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
      
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelBuildDesc.Inputs;
      bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
      bottomLevelInputs.Flags = buildFlags;
      bottomLevelInputs.NumDescs = 1;
      bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
      bottomLevelInputs.pGeometryDescs = &geometryDesc;
      
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
      topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
      topLevelInputs.Flags = buildFlags;
      topLevelInputs.NumDescs = 1;
      topLevelInputs.pGeometryDescs = nullptr;
      topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
      
      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
      device->com.Get()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
      THROW_IF_FALSE(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);
      
      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
      device->com.Get()->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
      THROW_IF_FALSE(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);
      
      fcom_ptr<ID3D12Resource> scratchResource;
      AllocateUAVBuffer(max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), &scratchResource, D3D12_RESOURCE_STATE_COMMON, L"ScratchResource");
      
      // Allocate resources for acceleration structures.
      // Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
      // Default heap is OK since the application doesn’t need CPU read/write access to them. 
      // The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
      // and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
      //  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
      //  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
      {
          D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
          
          AllocateUAVBuffer(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructure, initialResourceState, L"BottomLevelAccelerationStructure");
          AllocateUAVBuffer(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure, initialResourceState, L"TopLevelAccelerationStructure");
      }
      
      // Create an instance desc for the bottom-level acceleration structure.
      fcom_ptr<ID3D12Resource> instanceDescs;   
      D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
      instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
      instanceDesc.InstanceMask = 1;
      instanceDesc.AccelerationStructure = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
      AllocateUploadBuffer(&instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");
      
      // Bottom Level Acceleration Structure desc
      {
          bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
          bottomLevelBuildDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
      }
      
      // Top Level Acceleration Structure desc
      {
          topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
          topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
          topLevelBuildDesc.Inputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
      }
      
      // Build acceleration structure.
      command_list->com.Get()->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
      command_list->resource_uav_barrier(m_bottomLevelAccelerationStructure.Get());
      command_list->com.Get()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

      first_frame_done = true;
      return;
    }

    
  }
}