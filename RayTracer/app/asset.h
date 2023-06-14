#pragma once

#include <vector>
#include <string>

#include "app/factories.h"

// Persistent objects or those having resources on disk
// Base class for all assets, use like abstract
class asset
{
public:
  // Implement in child classes
  static asset_type get_static_asset_type();
  static asset* load(const std::string& name);

  void set_runtime_id(int id);
  int get_runtime_id() const;

  std::string get_asset_name() const;
  asset_type get_asset_type() const;

private:
  
  // Can be set only once by the registry, index in the vector
  // Can't change at runtime
  int runtime_id = -1;
};

// TODO exclude asset class, T needs to be a children
// 
// Soft asset pointer
// First get() call will trigger sync load and register asset
// Second get() call will return cached pointer
// No ref counting, no ownership
// set_name can be called multiple times with different values, this will invalidate existing pointer and load different asset
template<typename T>
struct soft_asset_ptr
{
  void set_name(const std::string& in_name)
  {
    if (in_name != name)
    {
      name = in_name;
      object = nullptr;
    }
  }

  std::string get_name() const
  {
    return name;
  }

  bool is_loaded() const
  {
    return object != nullptr;
  }

  const T* get() const
  {
    if (!is_loaded())
    {
      object = globals::get_asset_registry()->find_asset<T>(name);
      if (object == nullptr)
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

private:

  // Persistent name, or the one used to discovery on the disk
  // Can't change at runtime as I have no dependency update mechanism
  std::string name;

  mutable T* object = nullptr;
};


// Collection of assets of any kind.
// Registry has ownership on assets.
// Registry gives runtime ids.
// The fact that asset is in the registry should mean it has resources loaded.
// For now I assume there is no way to remove asset -> no defragmentation
// Assets can't be deleted from memory -> no dependence lookup or reference counting
class asset_registry
{
  // Only one instance allowed
  static bool created;

public:
  // No copy, no move
  asset_registry();
  ~asset_registry();
  asset_registry(const asset_registry&) = delete;
  asset_registry& operator=(const asset_registry&) = delete;

  bool is_valid(int id) const
  {
    return ((id >= 0 && id < assets.size()) && (assets[id] != nullptr));
  }

  // TODO exclude asset class, T needs to be a children
  template<typename T>
  void add(T* object, const std::string& name)
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
  T* get_asset(int id) const
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

  std::string get_name(int id) const;
  asset_type get_type(int id) const;

  template<typename T>
  T* find_asset(const std::string& name)
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
  std::vector<T*> get_assets()
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

  std::vector<int> get_ids(asset_type type) const;
  std::vector<std::string> get_names(asset_type type) const;

private:
  // Runtime is an index
  // None of this can change at runtime after is added
  std::vector<std::string> names;
  std::vector<asset_type> types;
  std::vector<asset*> assets;
};

