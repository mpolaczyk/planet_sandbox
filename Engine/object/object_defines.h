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
  static CLASS_NAME* spawn(const std::string& name); \
  static CLASS_NAME* load(const std::string& name); \
  static void save(CLASS_NAME* instance);

// Put this in the cpp file. Mandatory for every type.
#define OBJECT_DEFINE(CLASS_NAME, PARENT_CLASS_NAME) 
  
// Put this in the cpp file. Default implementations.
#define OBJECT_DEFINE_SPAWN(CLASS_NAME) CLASS_NAME* CLASS_NAME::spawn(const std::string& name) \
  { \
    CLASS_NAME* obj = new CLASS_NAME(); \
    bool success = get_object_registry()->add<CLASS_NAME>(obj, name); \
    return success ? obj : nullptr; \
  }

// Put this in the cpp file. Dummy plugs, use if object does not need the functionality.
#define OBJECT_DEFINE_NOSPAWN(CLASS_NAME) CLASS_NAME* CLASS_NAME::spawn(const std::string& name) { return nullptr; }
#define OBJECT_DEFINE_NOLOAD(CLASS_NAME) CLASS_NAME* CLASS_NAME::load(const std::string& name) { return nullptr; }
#define OBJECT_DEFINE_NOSAVE(CLASS_NAME) void CLASS_NAME::save(CLASS_NAME* object) { }