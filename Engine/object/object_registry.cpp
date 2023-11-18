
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

  std::vector<object*> object_registry::get_all()
  {
    return objects;
  }

  std::vector<int> object_registry::get_all_ids(object_type type) const
  {
    std::vector<int> ans;
    for (int i = 0; i < types.size(); i++)
    {
      if (types[i] == type)
      {
        ans.push_back(i);
      }
    }
    return ans;
  }

  std::vector<std::string> object_registry::get_all_names(object_type type) const
  {
    std::vector<std::string> ans;
    for (int i = 0; i < types.size(); i++)
    {
      if (types[i] == type)
      {
        ans.push_back(names[i]);
      }
    }
    return ans;
  }

}