#include "stdafx.h"

#include "deferred.h"

#include "passes/debug_pass.h"
#include "passes/deferred_lighting_pass.h"
#include "passes/gbuffer_pass.h"

#include "engine/renderer/command_list.h"

using namespace DirectX;

namespace engine
{
  // Because funique_ptr<forward declared type> requires destructor where the type is complete
  rdeferred::rdeferred() = default;
  rdeferred::~rdeferred() = default;

  OBJECT_DEFINE(rdeferred, rrenderer_base, Deferred renderer)
  OBJECT_DEFINE_SPAWN(rdeferred)
  OBJECT_DEFINE_VISITOR(rdeferred)

  bool rdeferred::init_passes()
  {
    if (!gbuffer_pass)
    {
      gbuffer_pass.reset(new fgbuffer_pass);
    }
    if (!deferred_lighting_pass)
    {
      deferred_lighting_pass.reset(new fdeferred_lighting_pass);
    }
    if (!debug_pass)
    {
      debug_pass.reset(new fdebug_pass);
    }
    return gbuffer_pass->init(&context) && deferred_lighting_pass->init(&context) && debug_pass->init(&context);
  }
  
  void rdeferred::draw_internal(fgraphics_command_list* command_list)
  {
    {
      gbuffer_pass->draw(&context, command_list);
    
      fresource_barrier_scope b(command_list, gbuffer_pass->position.com.Get(),    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      fresource_barrier_scope c(command_list, gbuffer_pass->normal.com.Get(),      D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      fresource_barrier_scope d(command_list, gbuffer_pass->uv.com.Get(),          D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      fresource_barrier_scope e(command_list, gbuffer_pass->material_id.com.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    
      deferred_lighting_pass->position = &gbuffer_pass->position;
      deferred_lighting_pass->normal = &gbuffer_pass->normal;
      deferred_lighting_pass->uv = &gbuffer_pass->uv;
      deferred_lighting_pass->material_id = &gbuffer_pass->material_id;
    
      deferred_lighting_pass->draw(&context, command_list);
    }
    
    debug_pass->blend_on = &deferred_lighting_pass->color;
    
    debug_pass->draw(&context, command_list);
  }  

  ftexture_resource* rdeferred::get_color()
  {
    return &deferred_lighting_pass->color;
  }

  ftexture_resource* rdeferred::get_depth()
  {
    return &gbuffer_pass->depth;
  }
}