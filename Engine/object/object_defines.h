#pragma once

// This file contains an interface that needs to be implemented for every managed object
// No RTTI, simple type detection for each object type.
// Type is stored in enum object_type.
// Root object should use itself as a parent class.

// Put this in the header file. Mandatory for every type.
#define OBJECT_DECLARE(CLASS_NAME, PARENT_CLASS_NAME) \
  virtual object_type get_type() const                { return object_type::CLASS_NAME; } \
  virtual object_type get_parent_type() const         { return object_type::PARENT_CLASS_NAME; } \
  inline static object_type get_type_static()         { return object_type::CLASS_NAME; } \
  inline static object_type get_parent_type_static()  { return object_type::PARENT_CLASS_NAME; } \
  virtual bool is_type(object_type type) const        { return CLASS_NAME::is_type_static(type); } \
  inline static bool is_type_static(object_type type) { return object_type::CLASS_NAME == object_type::PARENT_CLASS_NAME ? false : (type == object_type::CLASS_NAME || type == object_type::PARENT_CLASS_NAME || PARENT_CLASS_NAME::is_type_static(type)); } \
  static CLASS_NAME* spawn(const std::string& name); \
  static const class_object* get_class_static(); \
  static const class_object* get_parent_class_static(); \
  virtual const class_object* get_class() const; \
  virtual const class_object* get_parent_class() const; \
  virtual bool is_class(const class_object* type) const; \
  static bool is_class_static(const class_object* type);

// FIX only object registry can spawn or delete, no move no copy operators and ctors

// Put this in the cpp file. Mandatory for every type.
// Requires:
// #include "object/object_registry.h"
#define OBJECT_DEFINE(CLASS_NAME, PARENT_CLASS_NAME) \
  const class_object* CLASS_NAME::get_class_static()         { static class_object* cache = nullptr; if (cache == nullptr) { cache = get_object_registry()->find<class_object>(#CLASS_NAME); }        return cache; } \
  const class_object* CLASS_NAME::get_parent_class_static()  { static class_object* cache = nullptr; if (cache == nullptr) { cache = get_object_registry()->find<class_object>(#PARENT_CLASS_NAME); } return cache; } \
  const class_object* CLASS_NAME::get_class() const          { return get_class_static();        } \
  const class_object* CLASS_NAME::get_parent_class() const   { return get_parent_class_static(); } \
  bool CLASS_NAME::is_class(const class_object* type) const  { return CLASS_NAME::is_class_static(type); } \
  bool CLASS_NAME::is_class_static(const class_object* type) { return #CLASS_NAME == #PARENT_CLASS_NAME ? false : (type == CLASS_NAME::get_class_static() || type == PARENT_CLASS_NAME::get_class_static() || PARENT_CLASS_NAME::is_class_static(type)); }

// Put this in the cpp file. Default implementations.
#define OBJECT_DEFINE_SPAWN(CLASS_NAME) CLASS_NAME* CLASS_NAME::spawn(const std::string& name) \
  { \
    CLASS_NAME* obj = new CLASS_NAME(); \
    bool success = get_object_registry()->add<CLASS_NAME>(obj, name); \
    return success ? obj : nullptr; \
  }

// Put this in the cpp file. Dummy plugs, use if object does not need the functionality.
#define OBJECT_DEFINE_NOSPAWN(CLASS_NAME) CLASS_NAME* CLASS_NAME::spawn(const std::string& name) { return nullptr; }  // FIX assert

// Creates an instance of a class object for a given object type
#define REGISTER_CLASS_OBJECT(CLASS_NAME, PARENT_CLASS_NAME) \
  { \
    class_object* obj = class_object::spawn(#CLASS_NAME); \
    obj->class_name = #CLASS_NAME; \
    obj->parent_class_name = #PARENT_CLASS_NAME; \
  }