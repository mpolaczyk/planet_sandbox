#include "stdafx.h"

#include "app/asset.h"
#include "app/factories.h"


asset_type asset::get_static_asset_type()
{
  assert(false); // Not implemented!
  return asset_type::none;
}

asset* asset::load(const std::string& name)
{
  assert(false); // Not implemented!
  return nullptr;
}

void asset::save(asset* object)
{
  assert(false); // Not implemented!
}

asset* spawn()
{
  assert(false); // Not implemented!
  return nullptr;
}

std::string asset::get_display_name() const
{
  std::ostringstream oss;
  oss << "[" << runtime_id << "] " << asset_type_names[static_cast<int>(get_asset_type())] << ": " << get_asset_name();
  return oss.str();
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