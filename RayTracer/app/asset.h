#pragma once

#include <vector>
#include <string>

#include "engine.h"
#include <assert.h>

enum class asset_type : int
{
  none = 0,
  material,
  texture,
  static_mesh
};
static inline const char* asset_type_names[] =
{
  "None",
  "Material",
  "Texture",
  "Static Mesh"
};

// Persistent objects or those having resources on disk
// Base class for all assets, use like abstract
class asset
{
  friend asset_registry;

public:
  // Implement static functions in child classes!
  // They are not virtual members on purpose, most of the time when they are needed, there is no instance available
  static asset_type get_static_asset_type();
  static asset* load(const std::string& asset_name);
  static void save(asset* object);
  static asset* spawn();

  virtual std::string get_display_name() const;

  void set_runtime_id(int id);
  int get_runtime_id() const;

  std::string get_asset_name() const;
  asset_type get_asset_type() const;

private:
  
  // Can be set only once by the registry, index in the vector
  // Can't change at runtime, can't be cloned
  int runtime_id = -1;
};

struct soft_asset_ptr_base
{
  friend class soft_asset_ptr_base_serializer;

protected:
  // Persistent name, or the one used to discovery on the disk
  // Can't change at runtime as I have no dependency update mechanism
  std::string name;
};
// Soft asset pointer - persistent weak sync loading pointer to an asset
// First get() call will trigger sync load and register asset
// Second get() call will return cached pointer
// No ref counting, no ownership
// set_name can be called multiple times with different values, this will invalidate existing pointer and load different asset
template<typename T>
struct soft_asset_ptr : public soft_asset_ptr_base
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
        if (object != nullptr)
        {
          globals::get_asset_registry()->add<T>(object, name);
        }
        else
        {
          LOG_ERROR("Unable to find asset: {0}", name);
        }
      }
    }
    return object;
  }

private:

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

  template<typename T>
  bool add(T* object, const std::string& name)
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

  std::vector<asset*> get_all_assets()
  {
    return assets;
  }

  template<typename T>
  T* clone_asset(int source_runtime_id, const std::string& target_name)
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

  std::vector<int> get_ids(asset_type type) const;
  std::vector<std::string> get_names(asset_type type) const;

private:
  // Runtime id is an index
  // None of this can change at runtime after is added
  std::vector<std::string> names;
  std::vector<asset_type> types;
  std::vector<asset*> assets;
};
