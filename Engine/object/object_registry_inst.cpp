
#include "object/object_registry_temp.cpp"

#include "assets/mesh.h"
#include "assets/material.h"
#include "assets/texture.h"

#define OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(CLASS_NAME) \
  template ENGINE_API bool object_registry::add<CLASS_NAME>(CLASS_NAME* object, const std::string& name); \
  template ENGINE_API CLASS_NAME* object_registry::get<CLASS_NAME>(int id) const; \
  template ENGINE_API CLASS_NAME * object_registry::find<CLASS_NAME>(const std::string& name); \
  template ENGINE_API std::vector<CLASS_NAME*> object_registry::get_by_type<CLASS_NAME>(); \
  template ENGINE_API CLASS_NAME * object_registry::clone<CLASS_NAME>(int source_runtime_id, const std::string& target_name);

namespace engine
{
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(mesh)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(texture)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(material)
}

#undef OBJECT_REGISTRY_EXPLICIT_INSTANTIATE