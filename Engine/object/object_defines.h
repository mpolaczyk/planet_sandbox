#pragma once

#define OBJECT_DECLARE(CLASS_NAME) \
  static object_type get_static_type(); \
  static CLASS_NAME* spawn(); \
  static CLASS_NAME* load(const std::string& name); \
  static void save(CLASS_NAME* object);

#define OBJECT_DEFINE_BASE(CLASS_NAME) \
  object_type CLASS_NAME::get_static_type() { return object_type::CLASS_NAME; } \
  CLASS_NAME* CLASS_NAME::spawn() { return new CLASS_NAME(); } \

#define OBJECT_DEFINE_LOAD(CLASS_NAME) CLASS_NAME* CLASS_NAME::load(const std::string& name) { return asset_io::load_##CLASS_NAME(name); }
#define OBJECT_DEFINE_NOLOAD(CLASS_NAME) CLASS_NAME* CLASS_NAME::load(const std::string& name) { return nullptr; }

#define OBJECT_DEFINE_SAVE(CLASS_NAME) void CLASS_NAME::save(CLASS_NAME * object) { asset_io::save_##CLASS_NAME(object); }
#define OBJECT_DEFINE_NOSAVE(CLASS_NAME) void CLASS_NAME::save(CLASS_NAME* object) { }