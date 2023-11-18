#pragma once

// No RTTI, simple type detection for each object type

class async_renderer_base;
class hittable;

#include "engine.h"


class game_object_factory
{
public:

  // Utilities

  static async_renderer_base* spawn_renderer(renderer_type type);

  // Scene

  static hittable* spawn_hittable(hittable_type type);

};