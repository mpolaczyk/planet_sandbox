
#include "asset/asset_registry_temp.cpp"

#include "asset/mesh.h"
#include "asset/materials.h"
#include "asset/textures.h"

namespace engine
{
  template ENGINE_API bool object_registry::add<mesh>(mesh* object, const std::string& name);
  template ENGINE_API bool object_registry::add<material>(material* object, const std::string& name);
  template ENGINE_API bool object_registry::add<texture>(texture* object, const std::string& name);

  template ENGINE_API mesh* object_registry::get<mesh>(int id) const;
  template ENGINE_API material* object_registry::get<material>(int id) const;
  template ENGINE_API texture* object_registry::get<texture>(int id) const;

  template ENGINE_API mesh* object_registry::find<mesh>(const std::string& name);
  template ENGINE_API material* object_registry::find<material>(const std::string& name);
  template ENGINE_API texture* object_registry::find<texture>(const std::string& name);

  template ENGINE_API std::vector<mesh*> object_registry::get_by_type<mesh>();
  template ENGINE_API std::vector<material*> object_registry::get_by_type<material>();
  template ENGINE_API std::vector<texture*> object_registry::get_by_type<texture>();

  template ENGINE_API mesh* object_registry::clone<mesh>(int source_runtime_id, const std::string& target_name);
  template ENGINE_API material* object_registry::clone<material>(int source_runtime_id, const std::string& target_name);
  template ENGINE_API texture* object_registry::clone<texture>(int source_runtime_id, const std::string& target_name);
}