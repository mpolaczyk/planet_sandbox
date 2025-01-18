#include "stdafx.h"

#include "deferred_lighting_pass.h"

#include <vector>

#include "core/core.h"

#include "hittables/scene.h"

#include "assets/mesh.h"
#include "engine/math/math.h"
#include "engine/string_tools.h"

#include "engine/renderer/aligned_structs.h"
#include "engine/renderer/command_list.h"
#include "engine/renderer/render_context.h"
#include "engine/renderer/scene_acceleration.h"
#include "engine/renderer/device.h"
#include "engine/renderer/pipeline.h"

namespace engine
{
  using namespace DirectX;

  namespace
  {
    enum root_parameter_type : int
    {
      frame_data = 0,
      lights,
      materials,
      gbuffer_position,
      gbuffer_normal,
      gbuffer_uv,
      gbuffer_material_id,
      textures,
      num
    };

    DXGI_FORMAT rtv_formats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
    const char* rtv_names[1] = { "color"};
  }

  void fdeferred_lighting_pass::init_shaders()
  {
    pixel_shader_asset.set_name("lighting_deferred");
    vertex_shader_asset.set_name("lighting_deferred");
  }

  void fdeferred_lighting_pass::init_pipeline()
  {
    fpass_base::init_pipeline();
    const uint32_t num_textures = context->scene->scene_acceleration.get_num_textures();
    froot_signature sig;
    sig.reserve_parameters(root_parameter_type::num);
    // b
    sig.add_constant_parameter(root_parameter_type::frame_data, 0, 0, fmath::to_uint32(sizeof(fframe_data)), D3D12_SHADER_VISIBILITY_PIXEL);
    // t space0
    sig.add_shader_resource_view_parameter(root_parameter_type::lights, 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    sig.add_shader_resource_view_parameter(root_parameter_type::materials, 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    // t space1
    // TODO Convert gbuffer to one descriptor table parameter with 1 range and 4 descriptors. I failed to do it, something is wrong somewhere...
    // ID3D12Device::CreateRootSignature: RootParameterIndex [5] defines an empty root descriptor table. This may not have been intended, as it wastes space in the root signature. [ STATE_CREATION WARNING #1347: EMPTY_ROOT_DESCRIPTOR_TABLE]
    // Also, those textures can't be shader resource view parameters - SRV or UAV root descriptors can only be Raw or Structured buffers.
    sig.add_descriptor_table_parameter(root_parameter_type::gbuffer_position, 0, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
    sig.add_descriptor_table_parameter(root_parameter_type::gbuffer_normal, 1, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
    sig.add_descriptor_table_parameter(root_parameter_type::gbuffer_uv, 2, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
    sig.add_descriptor_table_parameter(root_parameter_type::gbuffer_material_id, 3, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
    // t space2
    sig.add_descriptor_table_parameter(root_parameter_type::textures, 0, 2, num_textures, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    // s
    pipeline->root_signature_rasterization = sig;
    
    pipeline->add_static_sampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
    pipeline->setup_formats(1, rtv_formats, DXGI_FORMAT_UNKNOWN);
    pipeline->init("Deferred lighting pass");
  }

  void fdeferred_lighting_pass::init_size_independent_resources()
  {
    uint32_t back_buffer_count = context->back_buffer_count;
    fdescriptor_heap* heap = context->main_descriptor_heap;
    fdevice* device = fapplication::get_instance()->device.get();

    // Create light and material data SRV
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fshader_resource_buffer buffer;
      device->create_shader_resource_buffer(heap, sizeof(flight_properties) * MAX_LIGHTS, buffer, fstring_tools::append("SRV lights: back buffer ", i).c_str());
      lights_data.emplace_back(buffer);
    }
    for(uint32_t i = 0; i < back_buffer_count; i++)
    {
      fshader_resource_buffer buffer;
      device->create_shader_resource_buffer(heap, sizeof(fmaterial_properties) * MAX_MATERIALS, buffer, fstring_tools::append("SRV materials: back buffer ", i).c_str());
      materials_data.emplace_back(buffer);
    }
    
    // Load and create quad mesh
    quad_asset.set_name("plane");
    quad_asset.get();
  }

  void fdeferred_lighting_pass::init_size_dependent_resources(bool cleanup)
  {
    fdevice* device = fapplication::get_instance()->device.get();
    
    color.release();
    device->create_frame_buffer(context->main_descriptor_heap, context->rtv_descriptor_heap, &color, context->width, context->height, rtv_formats[0], D3D12_RESOURCE_STATE_RENDER_TARGET, rtv_names[0]);
  }
  
  void fdeferred_lighting_pass::draw(frenderer_context* in_context, fcommand_list* command_list)
  {
    fpass_base::draw(in_context, command_list);
    
    fscene_acceleration& scene_acceleration = context->scene->scene_acceleration;
    ID3D12GraphicsCommandList* command_list_com = command_list->com.Get();
    const uint32_t back_buffer_index = context->back_buffer_index;

    // Clear and setup
    const float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    command_list->clear_render_target(&color, black);
    command_list->set_render_targets1(&color, nullptr);
    
    // Process frame data constants
    frame_data.camera_position = XMFLOAT4(context->scene->camera.location.e);
    frame_data.ambient_light = context->scene->ambient_light_color;
    frame_data.width = context->width;
    frame_data.height = context->height;

    // Process light and material SRVs
    {
      lights_data[back_buffer_index].upload(scene_acceleration.get_light_properties());
      materials_data[back_buffer_index].upload(scene_acceleration.get_material_properties());
    }
    
    // Upload quad mesh
    astatic_mesh* quad_mesh = quad_asset.get();
    if(!quad_mesh->is_resource_online)
    {
      command_list->upload_vertex_buffer(quad_mesh, "quad");
      command_list->upload_index_buffer(quad_mesh, "quad");
    }

    upload_all_textures_once(command_list);

    // Draw
    const fstatic_mesh_resource& smrs = quad_mesh->resource;
    command_list_com->SetGraphicsRoot32BitConstants(root_parameter_type::frame_data, fmath::to_uint32(sizeof(fframe_data))/4, &frame_data, 0);
    command_list_com->SetGraphicsRootShaderResourceView(root_parameter_type::lights, lights_data[back_buffer_index].resource->GetGPUVirtualAddress());
    command_list_com->SetGraphicsRootShaderResourceView(root_parameter_type::materials, materials_data[back_buffer_index].resource->GetGPUVirtualAddress());
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::gbuffer_position, position->srv.gpu_descriptor_handle);
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::gbuffer_normal, normal->srv.gpu_descriptor_handle);
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::gbuffer_uv, uv->srv.gpu_descriptor_handle);
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::gbuffer_material_id, material_id->srv.gpu_descriptor_handle);
    command_list_com->SetGraphicsRootDescriptorTable(root_parameter_type::textures, get_textures_gpu_handle());
    command_list_com->IASetVertexBuffers(0, 1, &smrs.vertex_buffer_view);
    command_list_com->IASetIndexBuffer(&smrs.index_buffer_view);
    command_list_com->DrawIndexedInstanced(smrs.vertex_num, 1, 0, 0, 0);
  }
}