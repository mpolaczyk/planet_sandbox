
#include "object/object_registry_temp.cpp"

#include "assets/mesh.h"
#include "assets/material.h"
#include "assets/texture.h"
#include "renderers/cpu_renderer_reference.h"
#include "renderers/cpu_renderer_preview.h"
#include "renderers/cpu_renderer_faces.h"
#include "renderers/cpu_renderer_normals.h"
#include "hittables/scene.h"
#include "hittables/sphere.h"
#include "hittables/static_mesh.h"

#define OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(CLASS_NAME) \
  template ENGINE_API bool object_registry::add<CLASS_NAME>(CLASS_NAME* instance, const std::string& name); \
  template ENGINE_API CLASS_NAME* object_registry::get<CLASS_NAME>(int id) const; \
  template ENGINE_API CLASS_NAME* object_registry::find<CLASS_NAME>(const std::string& name); \
  template ENGINE_API std::vector<CLASS_NAME*> object_registry::get_all_by_type<CLASS_NAME>(); \
  template ENGINE_API CLASS_NAME* object_registry::copy_shallow<CLASS_NAME>(const CLASS_NAME* source); \
  template ENGINE_API CLASS_NAME* object_registry::copy_shallow<CLASS_NAME>(const CLASS_NAME* source, const std::string& name);

namespace engine
{
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(static_mesh_asset)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(texture_asset)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(material_asset)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(cpu_renderer_reference)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(cpu_renderer_preview)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(cpu_renderer_faces)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(cpu_renderer_normals)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(hittable)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(scene)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(sphere)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(static_mesh)
    
}

#undef OBJECT_REGISTRY_EXPLICIT_INSTANTIATE