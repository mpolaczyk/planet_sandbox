
#include <assert.h>

#include "object/object_registry.h"
#include "engine/log.h"

namespace engine
{
  object_registry* g_object_registy = nullptr;

  object_registry* get_object_registry()
  {
    if (g_object_registy == nullptr)
    {
      g_object_registy = new object_registry();
    }
    return g_object_registy;
  }

  object_registry::~object_registry()
  {
    for (int i = 0; i < objects.size(); i++)
    {
      delete objects[i];
      objects[i] = nullptr;
    }
  }

  bool object_registry::is_valid(int id) const
  {
    return ((id >= 0 && id < objects.size()) && (objects[id] != nullptr));
  }

  std::string object_registry::get_custom_display_name(int id) const
  {
    assert(is_valid(id));
    return custom_display_names[id];
  }

  const class_object* object_registry::get_class(int id) const
  {
    assert(is_valid(id));
    return types[id];
  }

  void object_registry::destroy(int id)
  {
    // FIX don't destroy class objects
    assert(is_valid(id));
    delete objects[id];
    objects[id] = nullptr;
    custom_display_names[id] = "";
    types[id] = nullptr;
  }

  std::vector<object*> object_registry::get_all(bool no_nullptr)
  {
    // FIX don't get class objects
    // Warning, null objects may be filtered, indexes in the return vector will not match the runtime id
    std::vector<object*> ans;
    for (int i = 0; i < objects.size(); i++)
    {
      if (no_nullptr && !is_valid(i))
      {
        continue;
      }
      ans.push_back(objects[i]);
    }
    return ans;
  }

  std::vector<int> object_registry::get_all_ids(const class_object* type, bool no_nullptr) const
  {
    // FIX don't get class objects
    std::vector<int> ans;
    for (int i = 0; i < types.size(); i++)
    {
      if (no_nullptr && !is_valid(i))
      {
        continue;
      }
      if (types[i] == type)  // FIX is a child of
      {
        ans.push_back(i);
      }
    }
    return ans;
  }

  const class_object* object_registry::find_class(const std::string& name)
  {
    for (int i = 0; i < class_objects.size(); i++)
    {
      if(class_objects[i]->class_name == name)
      {
        return class_objects[i];
      }
    }
    return nullptr;
  }

  const class_object* object_registry::register_class(const std::string& class_name, const std::string& parent_class_name, spawn_instance_func_type spawn_func)
  {
    class_object* new_class = new class_object();
    new_class->class_name = class_name;
    new_class->parent_class_name = parent_class_name;
    new_class->spawn_instance_func = spawn_func;
    // Classes should not be registered twice, if this happens it is most likely a programmer error
    for (int i = 0; i < class_objects.size(); i++)
    {
      if (class_objects[i]->class_name == class_name)
      {
        LOG_ERROR("Unable to add class_object, it is already registered: {0}", class_name.c_str());
        return nullptr;
      }
    }
    new_class->set_runtime_id(objects.size());
    objects.push_back(new_class);
    class_objects.push_back(new_class);
    types.push_back(new_class);   // FIX ??
    custom_display_names.push_back("");
    return new_class;
  }
}