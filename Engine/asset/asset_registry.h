#pragma once

#include <string>
#include <vector>

#include "core/core.h"
#include "asset/asset.h"

namespace engine
{
  // Collection of assets of any kind.
  // Registry has ownership on assets.
  // Registry gives runtime ids.
  // The fact that asset is in the registry should mean it has resources loaded.
  // For now I assume there is no way to remove asset -> no defragmentation
  // Assets can't be deleted from memory -> no dependence lookup or reference counting
  // T can be only of type "asset" or derived from it
  class ENGINE_API asset_registry
  {
  public:
    // No copy, no move
    asset_registry() = default;
    ~asset_registry();
    asset_registry(const asset_registry&) = delete;
    asset_registry& operator=(const asset_registry&) = delete;

    bool is_valid(int id) const;
    std::string get_name(int id) const;
    asset_type get_type(int id) const;
    std::vector<asset*> get_all_assets();
    std::vector<int> get_ids(asset_type type) const;
    std::vector<std::string> get_names(asset_type type) const;

  protected:
    // Runtime id is an index
    // None of this can change at runtime after is added
    std::vector<std::string> names;
    std::vector<asset_type> types;
    std::vector<asset*> assets;

  public:

    template<typename T>
    bool add(T* object, const std::string& name);

    template<typename T>
    T* get_asset(int id) const;

    template<typename T>
    T* find_asset(const std::string& name);

    template<typename T>
    std::vector<T*> get_assets();

    template<typename T>
    T* clone_asset(int source_runtime_id, const std::string& target_name);
  };


  // Singleton
  ENGINE_API asset_registry* get_asset_registry();
}