#include "stdafx.h"

#include "app/asset.h"
#include "app/factories.h"
#include <sstream>




bool asset_registry::created = 0;

asset_registry::asset_registry()
{
  if (created)
  {
    assert(false, "Only one instance of asset registry is allowed");
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