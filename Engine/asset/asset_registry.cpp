
#include <assert.h>

#include "asset/asset_registry.h"
#include "engine/log.h"

namespace engine
{
  asset_registry* get_asset_registry()
  {
    static asset_registry* assets;
    if (assets == nullptr)
    {
      assets = new asset_registry();
    }
    return assets;
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

  std::vector<asset*> asset_registry::get_all_assets()
  {
    return assets;
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

}