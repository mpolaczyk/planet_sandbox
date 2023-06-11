#include "stdafx.h"

#include "app/asset_management.h"

template<typename T>
bool asset_instances<T>::is_id_in_use(const std::string& id) const
{
  if (id.size() == 0) return true;
  auto obj = registry.find(id);
  return obj != registry.end();
}

template<typename T>
bool asset_instances<T>::try_add(T* instance)
{
  if (instance == nullptr) return false;
  if (instance->id.size() == 0) return false;
  auto obj = registry.find(instance->id);
  if (obj != registry.end()) return false;
  registry.insert(std::pair<std::string, T*>(instance->id, instance));
  return true;
}

template<typename T>
void asset_instances<T>::remove(const std::string& id)
{
  auto obj = registry.find(id);
  if (obj != registry.end())
  {
    delete obj->second;
  }
  registry.erase(id);
}

template<typename T>
T* asset_instances<T>::get(const std::string& id) const
{
  auto obj = registry.find(id);
  if (obj != registry.end())
  {
    return obj->second;
  }
  return nullptr;
}

template<typename T>
std::vector<std::string> asset_instances<T>::get_ids() const
{
  std::vector<std::string> names;
  for (auto& pair : registry)
  {
    names.push_back(pair.first);
  }
  return names;
}

template<typename T>
std::vector<std::string> asset_instances<T>::get_names() const
{
  std::vector<std::string> names;
  for (auto& pair : registry)
  {
    std::string name;
    pair.second->get_name(name, false);
    names.push_back(name);
  }
  return names;
}

template<typename T>
int asset_instances<T>::get_index_by_name(const std::string& name) const
{
  std::vector<std::string> names = get_names();
  for (int i = 0; i < names.size(); i++)
  {
    if (names[i] == name)
    {
      return i;
    }
  }
  return -1;
}

template<typename T>
int asset_instances<T>::get_index_by_id(const std::string& id) const
{
  std::vector<std::string> ids = get_ids();
  for (int i = 0; i < ids.size(); i++)
  {
    if (ids[i] == id)
    {
      return i;
    }
  }
  return -1;
}

#include "math/materials.h"

template asset_instances<material>;