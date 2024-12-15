#pragma once

// This file contains an interface that needs to be implemented for every managed object
// No RTTI, simple type detection for each object type.
// Root object should use itself as a parent class.

#define MAX_INHERITANCE_DEPTH 5

// Put this in the header file. Mandatory for every type.
#define OBJECT_DECLARE(CLASS_NAME, PARENT_CLASS_NAME) \
  static CLASS_NAME* spawn(); \
  static const oclass_object* get_class_static(); \
  static const oclass_object* get_parent_class_static(); \
  virtual const oclass_object* get_class() const; \
  virtual const oclass_object* get_parent_class() const; \
  virtual bool is_child_of(const oclass_object* type) const; \
  static bool is_child_of_static(const oclass_object* type, int depth = MAX_INHERITANCE_DEPTH); \
  virtual std::string get_display_name() const; \
  virtual void set_display_name(const std::string& name);

#define OBJECT_DECLARE_VISITOR_BASE virtual void accept(struct vobject_visitor&& visitor);
#define OBJECT_DECLARE_VISITOR      virtual void accept(struct vobject_visitor&& visitor) override;

// Put this in the cpp file. Mandatory for every type.
// Requires:
// #include "core/rtti/object_registry.h"
#define OBJECT_DEFINE(CLASS_NAME, PARENT_CLASS_NAME, DEFAULT_DISPLAY_NAME) \
  const oclass_object* CLASS_NAME::get_class_static()         { static const oclass_object* cache = nullptr; if (cache == nullptr) { cache = REG.find_class(#CLASS_NAME); }        return cache; } \
  const oclass_object* CLASS_NAME::get_parent_class_static()  { static const oclass_object* cache = nullptr; if (cache == nullptr) { cache = REG.find_class(#PARENT_CLASS_NAME); } return cache; } \
  const oclass_object* CLASS_NAME::get_class() const          { return CLASS_NAME::get_class_static();        } \
  const oclass_object* CLASS_NAME::get_parent_class() const   { return CLASS_NAME::get_parent_class_static(); } \
  bool CLASS_NAME::is_child_of(const oclass_object* type) const  { return CLASS_NAME::is_child_of_static(type); } \
  bool CLASS_NAME::is_child_of_static(const oclass_object* type, int depth) \
  { \
    if (depth <= 0) { return false; } \
    if (type == oobject::get_class_static()) { return true; } \
    return type == CLASS_NAME::get_class_static() || type == PARENT_CLASS_NAME::get_class_static() || PARENT_CLASS_NAME::is_child_of_static(type, --depth); \
  } \
  std::string CLASS_NAME::get_display_name() const \
  { \
    std::string name = REG.get_custom_display_name(get_runtime_id()); \
    if (name.empty()) { return #DEFAULT_DISPLAY_NAME; } \
    return name; \
  } \
  void CLASS_NAME::set_display_name(const std::string& name) \
  { \
    REG.set_custom_display_name(get_runtime_id(), name); \
  }
// Put this in the cpp file. Default implementations.
#define OBJECT_DEFINE_SPAWN(CLASS_NAME) CLASS_NAME* CLASS_NAME::spawn() \
  { \
    CLASS_NAME* obj = new CLASS_NAME(); \
    bool success = REG.add<CLASS_NAME>(obj); \
    return success ? obj : nullptr; \
  }

// Put this in the cpp file. Dummy plugs, use if object does not need the functionality.
#define OBJECT_DEFINE_NOSPAWN(CLASS_NAME) CLASS_NAME* CLASS_NAME::spawn() { return nullptr; }

// Requires:
// #include "core/rtti/object_visitor.h"
#define OBJECT_DEFINE_VISITOR_BASE(CLASS_NAME) void CLASS_NAME::accept(vobject_visitor&& visitor) { }
#define OBJECT_DEFINE_VISITOR(CLASS_NAME) void CLASS_NAME::accept(vobject_visitor&& visitor) { visitor.visit(*this); }
