#pragma once

#include "renderer/cpu_renderer_base.h"
#include "hittables/hittables.h"
#include "core/core.h"

namespace engine
{
  class ENGINE_API object_factory
  {
  public:
    static cpu_renderer_base* spawn_renderer(const class_object* type);
    static hittable* spawn_hittable(const class_object* type);
  };
}