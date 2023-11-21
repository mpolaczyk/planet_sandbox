
#include <assert.h>

#include "object/object_registry.h"
#include "engine/log.h"

namespace engine
{
  object_registry* get_object_registry()
  {
    static object_registry* objects;
    if (objects == nullptr)
    {
      objects = new object_registry();
    }
    return objects;
  }

  object_registry::~object_registry()
  {
    for (int i = 0; i < objects.size(); i++)
    {
      delete objects[i];
      objects[i] = nullptr;
    }
    // FIX empty vectors
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

  object_type object_registry::get_type(int id) const
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
    types[id] = object_type::object;
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

  std::vector<int> object_registry::get_all_ids(object_type type, bool no_nullptr) const
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

  std::vector<std::string> object_registry::get_all_names(object_type type, bool no_nullptr) const
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

}