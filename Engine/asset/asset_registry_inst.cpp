
#include "asset/asset_registry_temp.cpp"

#include "asset/mesh.h"
#include "asset/materials.h"
#include "asset/textures.h"

namespace engine
{
  template ENGINE_API bool asset_registry::add<mesh>(mesh* object, const std::string& name);
  template ENGINE_API bool asset_registry::add<material>(material* object, const std::string& name);
  template ENGINE_API bool asset_registry::add<texture>(texture* object, const std::string& name);

  template ENGINE_API mesh* asset_registry::get_asset<mesh>(int id) const;
  template ENGINE_API material* asset_registry::get_asset<material>(int id) const;
  template ENGINE_API texture* asset_registry::get_asset<texture>(int id) const;

  template ENGINE_API mesh* asset_registry::find_asset<mesh>(const std::string& name);
  template ENGINE_API material* asset_registry::find_asset<material>(const std::string& name);
  template ENGINE_API texture* asset_registry::find_asset<texture>(const std::string& name);

  template ENGINE_API std::vector<mesh*> asset_registry::get_assets<mesh>();
  template ENGINE_API std::vector<material*> asset_registry::get_assets<material>();
  template ENGINE_API std::vector<texture*> asset_registry::get_assets<texture>();

  template ENGINE_API mesh* asset_registry::clone_asset<mesh>(int source_runtime_id, const std::string& target_name);
  template ENGINE_API material* asset_registry::clone_asset<material>(int source_runtime_id, const std::string& target_name);
  template ENGINE_API texture* asset_registry::clone_asset<texture>(int source_runtime_id, const std::string& target_name);
}