#pragma once

// This file contains a static interface that needs to be implemented for every managed object

// Put this in the header file
#define OBJECT_DECLARE(CLASS_NAME) \
  static object_type get_static_type(); \
  static CLASS_NAME* spawn(); \
  static CLASS_NAME* load(const std::string& name); \
  static void save(CLASS_NAME* object);

// Put this in the cpp file
#define OBJECT_DEFINE_BASE(CLASS_NAME) \
  object_type CLASS_NAME::get_static_type() { return object_type::CLASS_NAME; } \
  CLASS_NAME* CLASS_NAME::spawn() { return new CLASS_NAME(); } \

// Put this in the cpp file, implement manually if object needs persistence
#define OBJECT_DEFINE_NOLOAD(CLASS_NAME) CLASS_NAME* CLASS_NAME::load(const std::string& name) { return nullptr; }

// Put this in the cpp file, implement manually if object needs persistence
#define OBJECT_DEFINE_NOSAVE(CLASS_NAME) void CLASS_NAME::save(CLASS_NAME* object) { }