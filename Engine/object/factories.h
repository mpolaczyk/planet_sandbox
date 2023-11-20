#pragma once

// No RTTI, simple type detection for each object type


#include "renderer/async_renderer_base.h"
#include "hittables/hittables.h"
#include "core/core.h"


namespace engine
{
  class ENGINE_API object_factory
  {
  public:
    static async_renderer_base* spawn_renderer(renderer_type type);
    static hittable* spawn_hittable(hittable_type type);
  };
}