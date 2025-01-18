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

  void fray_tracing_pass::init_size_independent_resources()
  {
    
  }
  
  void fray_tracing_pass::init_size_dependent_resources(bool cleanup)
  {
    // Nothing to do here
  }

  void fray_tracing_pass::draw(frenderer_context* in_context, fcommand_list* command_list)
  {
    fpass_base::draw(in_context, command_list);
    
    
  }
}