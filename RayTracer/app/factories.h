#pragma once

// No RTTI, simple type detection for each object type

class material;
class async_renderer_base;
class hittable;
class texture;
class material_instances;

template<typename T>
class asset_registry;

enum class material_type
{
  none = 0,
  universal,
  light
};
static inline const char* material_type_names[] =
{
  "None",
  "Universal",
  "Light"
};

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

enum class texture_type
{
  none = 0,
  solid,
  checker,
  asset
};
static inline const char* texture_type_names[] =
{
  "None",
  "Solid",
  "Checker",
  "Asset"
};

class object_factory
{
public:
  static material* spawn_material(material_type type);

  static async_renderer_base* spawn_renderer(renderer_type type);

  static hittable* spawn_hittable(hittable_type type);

  static texture* spawn_texture(texture_type type);

  static asset_registry<material>* spawn_material_instances();

};