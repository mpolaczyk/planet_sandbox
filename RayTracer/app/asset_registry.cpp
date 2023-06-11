#include "stdafx.h"

#include "app/asset_registry.h"


#include "math/materials.h"
#include "math/textures.h"

template asset_registry<std::string, material>;
//template asset_registry<texture>;

template<typename K, typename V>
bool asset_registry<K,V>::is_id_in_use(const K& id) const
{
  if (id.size() == 0) return true;
  auto obj = registry.find(id);
  return obj != registry.end();
}

template<typename K, typename V>
bool asset_registry<K, V>::try_add(V* instance)
{
  if (instance == nullptr) return false;
  if (instance->id.size() == 0) return false;
  auto obj = registry.find(instance->id);
  if (obj != registry.end()) return false;
  registry.insert(std::pair<K, V*>(instance->id, instance));
  return true;
}

template<typename K, typename V>
void asset_registry<K, V>::remove(const K& id)
{
  auto obj = registry.find(id);
  if (obj != registry.end())
  {
    delete obj->second;
  }
  registry.erase(id);
}

template<typename K, typename V>
V* asset_registry<K, V>::get(const K& id) const
{
  auto obj = registry.find(id);
  if (obj != registry.end())
  {
    return obj->second;
  }
  return nullptr;
}

template<typename K, typename V>
std::vector<K> asset_registry<K, V>::get_ids() const
{
  std::vector<K> names;
  for (auto& pair : registry)
  {
    names.push_back(pair.first);
  }
  return names;
}

template<typename K, typename V>
std::vector<K> asset_registry<K, V>::get_names() const
{
  std::vector<K> names;
  for (auto& pair : registry)
  {
    names.push_back(pair.second->get_name());
  }
  return names;
}

template<typename K, typename V>
int asset_registry<K, V>::get_index_by_name(const K& name) const
{
  std::vector<K> names = get_names();
  for (int i = 0; i < names.size(); i++)
  {
    if (names[i] == name)
    {
      return i;
    }
  }
  return -1;
}

template<typename K, typename V>
int asset_registry<K, V>::get_index_by_id(const K& id) const
{
  std::vector<K> ids = get_ids();
  for (int i = 0; i < ids.size(); i++)
  {
    if (ids[i] == id)
    {
      return i;
    }
  }
  return -1;
}
