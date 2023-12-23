
#include <cassert>

#include "object/object_registry.h"
#include "engine/log.h"

namespace engine
{
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
    return object_custom_display_names[id];
  }

  void object_registry::set_custom_display_name(int id, const std::string& name)
  {
    assert(is_valid(id));
    object_custom_display_names[id] = name;
  }

  const class_object* object_registry::get_class(int id) const
  {
    assert(is_valid(id));
    return object_classes[id];
  }

  void object_registry::destroy(int id)
  {
    assert(is_valid(id));
    delete objects[id];
    objects[id] = nullptr;
    object_custom_display_names[id] = "";
    object_classes[id] = nullptr;
  }

  std::vector<object*> object_registry::get_all(bool no_nullptr)
  {
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
    std::vector<int> ans;
    for (int i = 0; i < object_classes.size(); i++)
    {
      if (no_nullptr && !is_valid(i))
      {
        continue;
      }
      if (object_classes[i]->is_child_of(type))
      {
        ans.push_back(i);
      }
    }
    return ans;
  }

  const class_object* object_registry::find_class(const std::string& name) const
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

  std::vector<const class_object*> object_registry::get_classes() const
  {
    return class_objects;
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
        LOG_ERROR("Unable to add class_object, it is already registered: {0}", class_name);
        return nullptr;
      }
    }
    new_class->set_runtime_id(static_cast<int>(objects.size()));
    objects.push_back(new_class);
    class_objects.push_back(new_class);
    object_classes.push_back(new_class);
    object_custom_display_names.push_back(class_name);
    return new_class;
  }
}