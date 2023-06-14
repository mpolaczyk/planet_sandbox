#include "stdafx.h"

#include <algorithm>

#include "app/asset.h"
#include "app/factories.h"

// TODO move explicit instantiation somewhere else!
#include "math/materials.h"
template class soft_asset_ptr<material>;
template void asset_registry::add<material>(material* object, const std::string& name);
template material* asset_registry::get_asset(int id) const;
template material* asset_registry::find_asset(const std::string& name);
template std::vector<material*> asset_registry::get_assets();


asset_type asset::get_static_asset_type()
{
  return asset_type::none;
}

asset* asset::load(const std::string& name)
{
  return nullptr;
}

void asset::set_runtime_id(int id)
{
  if (runtime_id == -1)
  {
    runtime_id = id;
  }
}

int asset::get_runtime_id() const
{
  return runtime_id;
}

std::string asset::get_asset_name() const
{
  return globals::get_asset_registry()->get_name(runtime_id);
}

asset_type asset::get_asset_type() const
{
  return globals::get_asset_registry()->get_type(runtime_id);
}




template<typename T>
bool soft_asset_ptr<T>::is_loaded() const
{
  return object != nullptr;
}

template<typename T>
void soft_asset_ptr<T>::set_name(const std::string& in_name)
{
  if (in_name != name)
  {
    name = in_name;
    object = nullptr;
  }
}

template<typename T>
std::string soft_asset_ptr<T>::get_name() const
{
  return name;
}

template<typename T>
const T* soft_asset_ptr<T>::get() const
{
  if (!is_loaded())
  {
    object = globals::get_asset_registry()->find_asset<T>(name);
    if(object == nullptr)
    {
      object = T::load(name);
      globals::get_asset_registry()->add<T>(object, name);
    }
    if (object == nullptr)
    {
      logger::critical("Unable to find asset: {0}", name);
      // TODO future: use default engine material
    }
  }
  return object;
}




bool asset_registry::created = 0;

asset_registry::asset_registry()
{
  if (created)
  {
    assert("Only one instance of asset registry is allowed");
  }
  created = true;
}

asset_registry::~asset_registry()
{
  for (int i = 0; i < assets.size(); i++)
  {
    delete assets[i];
    assets[i] = nullptr;
  }
}

bool asset_registry::is_valid(int id) const
{
  return ((id >= 0 && id < assets.size()) && (assets[id] != nullptr));
}

template<typename T>
void asset_registry::add(T* object, const std::string& name)
{
  if (object->get_static_asset_type() == asset_type::none)
  {
    assert(false, "Unable to add none object.");
    return;
  }
  if (object == nullptr)
  {
    assert(false, "Unable to add nullptr object.");
    return;
  }
  if (std::find(begin(assets), end(assets), object) != end(assets))
  {
    assert(false, "Unable to add object, it is already registered.");
    return;
  }
  object->set_runtime_id(assets.size());
  assets.push_back(object);
  names.push_back(name);
  types.push_back(T::get_static_asset_type());
}

template<typename T>
T* asset_registry::get_asset(int id) const
{
  if(is_valid(id))
  {
    if (get_type(id) == T::get_static_asset_type())
    {
      return static_cast<T*>(assets[id]); // Risky! no RTTI, no dynamic_cast
    }
  }
  return nullptr;
}

std::string asset_registry::get_name(int id) const
{
  assert(is_valid(id));
  return names[id];
}

asset_type asset_registry::get_type(int id) const
{
  assert(is_valid(id));
  return types[id];
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

std::vector<int> asset_registry::get_ids(asset_type type) const
{
  std::vector<int> ans;
  for (int i = 0; i < types.size(); i++)
  {
    if (types[i] == type)
    {
      ans.push_back(i);
    }
  }
  return ans;
}

std::vector<std::string> asset_registry::get_names(asset_type type) const
{
  std::vector<std::string> ans;
  for (int i = 0; i < types.size(); i++)
  {
    if (types[i] == type)
    {
      ans.push_back(names[i]);
    }
  }
  return ans;
}