#pragma once

namespace engine
{
  // Managed object types
  // Don't change the order as values can be persistent.
  // TODO: I need a auto generated type class in the future so I don't have to use this enum for everything!
  // TODO: Persistent class pointer so that enum values does not have to be serialized
  //enum class object_type : int
  //{
  //  object = 0,
  //  material_asset = 1,
  //  texture_asset,
  //  static_mesh_asset,
  //  asset_base,
  //  cpu_renderer_base = 5,
  //  cpu_renderer_preview,
  //  cpu_renderer_normals,
  //  cpu_renderer_faces,
  //  cpu_renderer_reference,
  //  hittable = 10,
  //  scene,
  //  static_mesh,
  //  sphere,
  //  class_object = 14
  //};
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
    "CPU renderer: reference",
    "Hittable",
    "Scene",
    "Static Mesh",
    "Sphere",
    "Class object"
  };
}