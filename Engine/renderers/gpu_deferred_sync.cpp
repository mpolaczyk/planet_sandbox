
#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "gpu_deferred_sync.h"

#include "renderer/command_list.h"

using namespace DirectX;

namespace engine
{
  OBJECT_DEFINE(rgpu_deferred_sync, rrenderer_base, GPU deferred sync)
  OBJECT_DEFINE_SPAWN(rgpu_deferred_sync)
  OBJECT_DEFINE_VISITOR(rgpu_deferred_sync)

  bool rgpu_deferred_sync::init_passes()
  {
    return gbuffer_pass.init(&context) && deferred_lighting_pass.init(&context);
  }
  
  void rgpu_deferred_sync::draw_internal(fgraphics_command_list* command_list)
  {
    // GBuffer pass
    {
      gbuffer_pass.draw(&context, command_list);
    }

    // Deferred lighting pass
    {
      fresource_barrier_scope b(command_list, gbuffer_pass.position.com.Get(),    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      fresource_barrier_scope c(command_list, gbuffer_pass.normal.com.Get(),      D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      fresource_barrier_scope d(command_list, gbuffer_pass.uv.com.Get(),          D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      fresource_barrier_scope e(command_list, gbuffer_pass.material_id.com.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      
      deferred_lighting_pass.position = &gbuffer_pass.position;
      deferred_lighting_pass.normal = &gbuffer_pass.normal;
      deferred_lighting_pass.uv = &gbuffer_pass.uv;
      deferred_lighting_pass.material_id = &gbuffer_pass.material_id;
      
      deferred_lighting_pass.draw(&context, command_list);
    }
  }

  ftexture_resource* rgpu_deferred_sync::get_color()
  {
    return &deferred_lighting_pass.color;
  }

  ftexture_resource* rgpu_deferred_sync::get_depth()
  {
    return &gbuffer_pass.depth;
  }
}