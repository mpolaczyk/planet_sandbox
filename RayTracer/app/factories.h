#pragma once

// No RTTI, simple type detection for each object type

class async_renderer_base;
class hittable;

#include "engine.h"


enum class renderer_type
{
  example = 0,
  preview,
  preview_normals,
  preview_faces,
  reference,
  ispc
};
static inline const char* renderer_type_names[] =
{
  "CPU Example",
  "CPU Preview",
  "CPU Preview Normals",
  "CPU Preview Faces",
  "CPU Reference",
  "CPU ISPC (Example only)"
};

enum class hittable_type
{
  scene = 0,
  sphere,
  xy_rect,
  xz_rect,
  yz_rect,
  static_mesh
};
static inline const char* hittable_type_names[] =
{
  "Scene",
  "Sphere",
  "XY Rectangle",
  "XZ Rectangle",
  "YZ Rectangle",
  "Static Mesh"
};


class object_factory
{
public:

  // Utilities

  static async_renderer_base* spawn_renderer(renderer_type type);

  // Scene

  static hittable* spawn_hittable(hittable_type type);

  // Resources 

  static engine::material* spawn_material(material_type type);

  static engine::mesh* spawn_mesh();

  static engine::texture* spawn_texture();
};