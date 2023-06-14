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
  void set_name(const std::string& in_name);
  std::string get_name() const;

  bool is_loaded() const;

  const T* get() const;

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

  bool is_valid(int id) const;

  // TODO exclude asset class, T needs to be a children
  template<typename T>
  void add(T* object, const std::string& name);

  template<typename T>
  T* get_asset(int id) const;

  std::string get_name(int id) const;
  asset_type get_type(int id) const;

  template<typename T>
  T* find_asset(const std::string& name);

  template<typename T>
  std::vector<T*> get_assets();

  std::vector<int> get_ids(asset_type type) const;
  std::vector<std::string> get_names(asset_type type) const;

private:
  // Runtime is an index
  // None of this can change at runtime after is added
  std::vector<std::string> names;
  std::vector<asset_type> types;
  std::vector<asset*> assets;
};

