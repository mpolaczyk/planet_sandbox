#pragma once

namespace engine
{
  // Don't change the order as values can be persistent
  enum class object_type : int
  {
    object = 0,
    material_asset,
    texture_asset,
    static_mesh_asset
  };
  static inline const char* object_type_names[] =
  {
    "None",
    "Material asset",
    "Texture asset",
    "Static mesh asset"
  };

  // Don't change the order as values can be persistent
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

  // Don't change the order as values can be persistent
  enum class renderer_type
  {
    example = 0,
    preview,
    preview_normals,
    preview_faces,
    reference,
    num
  };
  static inline const char* renderer_type_names[] =
  {
    "CPU Example",
    "CPU Preview",
    "CPU Preview Normals",
    "CPU Preview Faces",
    "CPU Reference"
  };

  // Don't change the order as values can be persistent
  enum class hittable_type
  {
    scene = 0,
    static_mesh,
    sphere,
    num
  };
  static inline const char* hittable_type_names[] =
  {
    "Scene",
    "Static Mesh",
    "Sphere"
  };
}