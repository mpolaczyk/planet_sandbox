
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

  std::string object_registry::get_name(int id) const
  {
    assert(is_valid(id));
    return names[id];
  }

  const class_object* object_registry::get_class(int id) const
  {
    assert(is_valid(id));
    return types[id];
  }

  void object_registry::destroy(int id)
  {
    assert(is_valid(id));
    delete objects[id];
    objects[id] = nullptr;
    names[id] = "";
    types[id] = nullptr;
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

  std::vector<std::string> object_registry::get_all_names(const class_object* type, bool no_nullptr) const
  {
    // Warning, null objects may be filtered, indexes in the return vector will not match the runtime id
    std::vector<std::string> ans;
    for (int i = 0; i < types.size(); i++)
    {
      if (no_nullptr && !is_valid(i))
      {
        continue;
      }
      if (types[i] == type)   // FIX is a child of
      {
        ans.push_back(names[i]);
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
    LOG_ERROR("Unable to find class object by name: {0}", name.c_str());
    return nullptr;
  }

  void object_registry::register_class(class_object* instance)
  {
    if (instance == nullptr)
    {
      LOG_ERROR("Unable to add nullptr class_object.");
      return;
    }
    if (instance->runtime_id != -1)
    {
      LOG_ERROR("Unable to add already added class_object.");
      return;
    }
    // Objects should not be added twice, if this happens it is most likely a programmer error
    if (std::find(begin(objects), end(objects), instance) != end(objects))
    {
      LOG_ERROR("Unable to add class_object, it is already registered: {0}", instance->class_name.c_str());
      return;
    }
    instance->set_runtime_id(objects.size());
    // Make name unique if exists
    std::string final_name = instance->class_name;
    if (std::find(begin(names), end(names), final_name) != end(names))
    {
      final_name.append(std::to_string(instance->runtime_id));
    }
    objects.push_back(instance);
    names.push_back(final_name);
    types.push_back(instance);   // FIX ??
    class_objects.push_back(instance);
  }
}