#pragma once

namespace engine
{
  enum class object_type : int
  {
    none = 0,
    material,
    texture,
    static_mesh
  };
  static inline const char* object_type_names[] =
  {
    "None",
    "Material",
    "Texture",
    "Static Mesh"
  };

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
}