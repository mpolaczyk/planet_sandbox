#pragma once

#include "core/core.h"

#include "asset/object_types.h"

#include <string>

namespace engine
{
#define OBJECT_DECLARE(CLASS_NAME) \
  static object_type get_static_type(); \
  static CLASS_NAME* spawn(); \
  static CLASS_NAME* load(const std::string& name); \
  static void save(CLASS_NAME* object);

#define OBJECT_DEFINE_BASE(CLASS_NAME) \
  object_type CLASS_NAME::get_static_type() { return object_type::CLASS_NAME; } \
  CLASS_NAME* CLASS_NAME::spawn() { return new CLASS_NAME(); } \

#define OBJECT_DEFINE_LOAD(CLASS_NAME) CLASS_NAME* CLASS_NAME::load(const std::string& name) { return asset_discovery::load_##CLASS_NAME(name); }
#define OBJECT_DEFINE_NOLOAD(CLASS_NAME) CLASS_NAME* CLASS_NAME::load(const std::string& name) { return nullptr; }

#define OBJECT_DEFINE_SAVE(CLASS_NAME) void CLASS_NAME::save(CLASS_NAME * object) { asset_discovery::save_##CLASS_NAME(object); }
#define OBJECT_DEFINE_NOSAVE(CLASS_NAME) void CLASS_NAME::save(CLASS_NAME* object) { }


  // Managed object class
  // Base class for all objects, use like abstract
  class ENGINE_API object
  {
    //friend asset_registry; // FIX

  public:
    OBJECT_DECLARE(object)

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