#pragma once

#include "core/core.h"

#include <string>

namespace engine
{
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
  class ENGINE_API asset
  {
    //friend asset_registry; // FIX

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

  //private:  // FIX

    // Can be set only once by the registry, index in the vector
    // Can't change at runtime, can't be cloned
    int runtime_id = -1;
  };
}