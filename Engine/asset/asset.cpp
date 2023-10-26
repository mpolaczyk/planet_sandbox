
#include <assert.h>
#include <sstream>  

#include "asset/asset.h"

namespace engine
{
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
    //return globals::get_asset_registry()->get_name(runtime_id);  // FIX
    return "";
  }

  asset_type asset::get_asset_type() const
  {
    //return globals::get_asset_registry()->get_type(runtime_id); // FIX
    return asset_type::none;
  }
}