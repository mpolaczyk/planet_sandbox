
#include <assert.h>

#include "asset/asset_registry.h"
#include "engine/log.h"

// Those definitions are moved away from asset_registry.cpp to avoid LNK2005 error, when compiling asset_registry_inst.cpp
// https://stackoverflow.com/questions/77377405/lnk2005-for-non-templated-function-when-explicitly-instantiating-templated-funct

namespace engine
{
  template<typename T>
  bool asset_registry::add(T* object, const std::string& name)
  {
    if (object->get_static_asset_type() == asset_type::none)
    {
      assert(false); // "Unable to add none object."
      return false;
    }
    if (object == nullptr)
    {
      assert(false); // "Unable to add nullptr object."
      return false;
    }
    // Assets should not be added twice, if this happens it is most likely a programmer error
    if (std::find(begin(assets), end(assets), object) != end(assets))
    {
      LOG_ERROR("Unable to add asset, it is already registered: {0}", name.c_str());
      return false;
    }
    if (std::find(begin(names), end(names), name) != end(names))
    {
      LOG_ERROR("Unable to add asset, name is already registered. {0}", name.c_str());
      return false;
    }
    object->set_runtime_id(assets.size());
    assets.push_back(object);
    names.push_back(name);
    types.push_back(T::get_static_asset_type());
    return true;
  }

  template<typename T>
  T* asset_registry::get_asset(int id) const
  {
    if (is_valid(id))
    {
      if (get_type(id) == T::get_static_asset_type())
      {
        return static_cast<T*>(assets[id]); // Risky! no RTTI, no dynamic_cast
      }
    }
    return nullptr;
  }

  template<typename T>
  T* asset_registry::find_asset(const std::string& name)
  {
    for (int i = 0; i < types.size(); i++)
    {
      if (types[i] == T::get_static_asset_type() && names[i] == name)
      {
        assert(assets[i] != nullptr);
        return static_cast<T*>(assets[i]); // Risky! no RTTI, no dynamic_cast
      }
    }
    return nullptr;
  }

  template<typename T>
  std::vector<T*> asset_registry::get_assets()
  {
    std::vector<T*> ans;
    for (int i = 0; i < types.size(); i++)
    {
      if (types[i] == T::get_static_asset_type())
      {
        assert(assets[i] != nullptr);
        ans.push_back(static_cast<T*>(assets[i])); // Risky! no RTTI, no dynamic_cast
      }
    }
    return ans;
  }

  template<typename T>
  T* asset_registry::clone_asset(int source_runtime_id, const std::string& target_name)
  {
    if (!is_valid(source_runtime_id))
    {
      LOG_ERROR("Unable to clone asset: {0} Unknown source runtime id: {1}", target_name.c_str(), source_runtime_id);
      return nullptr;
    }
    if (types[source_runtime_id] != T::get_static_asset_type())
    {
      LOG_ERROR("Unable to clone asset: {0} Type mismatch: {1} and {2}", target_name.c_str(), asset_type_names[static_cast<int>(types[source_runtime_id])], asset_type_names[static_cast<int>(T::get_static_asset_type())]);
      return nullptr;
    }
    T* source = static_cast<T*>(assets[source_runtime_id]);  // Risky! no RTTI, no dynamic_cast
    if (source == nullptr)
    {
      LOG_ERROR("Unable to clone asset: {0} Invalid source object: {1}", target_name.c_str(), source_runtime_id);
      return nullptr;
    }
    // Shallow copy
    T* obj = T::spawn();
    *obj = *source;
    obj->runtime_id = -1;

    // Register new one
    if (add<T>(obj, target_name))
    {
      return obj;
    }
    return nullptr;
  }


}