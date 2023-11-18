#pragma once

#include "core/core.h"

#include "asset/object_types.h"

#include <string>

namespace engine
{
  // Managed object class
  // Base class for all objects, use like abstract
  class ENGINE_API object
  {
    //friend asset_registry; // FIX

  public:
    // Implement static functions in child classes!
    // They are not virtual members on purpose, most of the time when they are needed, there is no instance available
    static object_type get_static_type();
    static object* load(const std::string& object_name);
    static void save(object* object);
    static object* spawn();

    virtual std::string get_display_name() const;

    void set_runtime_id(int id);
    int get_runtime_id() const;

    std::string get_name() const;
    object_type get_type() const;

  //private:  // FIX

    // Can be set only once by the registry, index in the vector
    // Can't change at runtime, can't be cloned
    int runtime_id = -1;
  };
}