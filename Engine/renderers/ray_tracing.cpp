#include "stdafx.h"

#include "ray_tracing.h"

#include "passes/ray_tracing_pass.h"

#include "engine/renderer/command_list.h"

namespace engine
{
  // Because funique_ptr<forward declared type> requires destructor where the type is complete
  rray_tracing::rray_tracing() = default;
  rray_tracing::~rray_tracing() = default;

  OBJECT_DEFINE(rray_tracing, rrenderer_base, Ray tracing renderer)
  OBJECT_DEFINE_SPAWN(rray_tracing)
  OBJECT_DEFINE_VISITOR(rray_tracing)
  
  bool rray_tracing::init_passes()
  {
    if (!ray_tracing_pass)
    {
      ray_tracing_pass.reset(new fray_tracing_pass);
    }
    return ray_tracing_pass->init(&context);
  }

  void rray_tracing::draw_internal(fcommand_list* command_list)
  {
    ray_tracing_pass->draw(&context, command_list);
  }

  ftexture_resource* rray_tracing::get_color()
  {
    return &ray_tracing_pass->color;
  }

  ftexture_resource* rray_tracing::get_depth()
  {
    return &ray_tracing_pass->color;  // TODO no depth
  }
}