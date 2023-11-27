#pragma once

// This file contains an interface that needs to be implemented for every managed object
// No RTTI, simple type detection for each object type.
// Type is stored in enum object_type.
// Root object should use itself as a parent class.

// Put this in the header file. Mandatory for every type.
#define OBJECT_DECLARE(CLASS_NAME, PARENT_CLASS_NAME) \
  virtual object_type get_type() const                { return object_type::CLASS_NAME; } \
  virtual object_type get_parent_type() const         { return object_type::PARENT_CLASS_NAME; } \
  virtual bool is_type(object_type type) const        { return CLASS_NAME::is_type_static(type); } \
  inline static object_type get_type_static()         { return object_type::CLASS_NAME; } \
  inline static object_type get_parent_type_static()  { return object_type::PARENT_CLASS_NAME; } \
  inline static bool is_type_static(object_type type) { return object_type::CLASS_NAME == object_type::PARENT_CLASS_NAME ? false : (type == object_type::CLASS_NAME || type == object_type::PARENT_CLASS_NAME || PARENT_CLASS_NAME::is_type_static(type)); } \
  static CLASS_NAME* spawn(const std::string& name);
// FIX only object registry can spawn or delete, no move no copy operators and ctors

// Put this in the cpp file. Mandatory for every type.
// Requires:
// #include "object/object.h"
#define OBJECT_DEFINE(CLASS_NAME, PARENT_CLASS_NAME) 
  
// Put this in the cpp file. Default implementations.
// Requires:
// #include "object/object_registry.h"
#define OBJECT_DEFINE_SPAWN(CLASS_NAME) CLASS_NAME* CLASS_NAME::spawn(const std::string& name) \
  { \
    CLASS_NAME* obj = new CLASS_NAME(); \
    bool success = get_object_registry()->add<CLASS_NAME>(obj, name); \
    return success ? obj : nullptr; \
  }

// Put this in the cpp file. Dummy plugs, use if object does not need the functionality.
#define OBJECT_DEFINE_NOSPAWN(CLASS_NAME) CLASS_NAME* CLASS_NAME::spawn(const std::string& name) { return nullptr; }