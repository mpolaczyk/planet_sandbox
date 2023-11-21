#pragma once

namespace engine
{
  // Managed object types
  // Don't change the order as values can be persistent
  enum class object_type : int
  {
    object = 0,
    material_asset,
    texture_asset,
    static_mesh_asset,
    asset_base,
    cpu_renderer_base,
    cpu_renderer_preview,
    cpu_renderer_normals,
    cpu_renderer_faces,
    cpu_renderer_reference
  };
  static inline const char* object_type_names[] =
  {
    "Object",
    "Material asset",
    "Texture asset",
    "Static mesh asset",
    "Asset base",
    "CPU renderer base",
    "CPU renderer: preview",
    "CPU renderer: normals",
    "CPU renderer: faces",
    "CPU renderer: reference"
  };


  // FIX Move objects below to the managed object types
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