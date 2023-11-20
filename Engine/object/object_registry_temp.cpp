
#include <assert.h>

#include "object/object_registry.h"
#include "engine/log.h"

// Those definitions are moved away from object_registry.cpp to avoid LNK2005 error, when compiling object_registry_inst.cpp
// https://stackoverflow.com/questions/77377405/lnk2005-for-non-templated-function-when-explicitly-instantiating-templated-funct

namespace engine
{
  template<derives_from<object> T>
  bool object_registry::add(T* instance, const std::string& name)
  {
    if (instance->get_static_type() == object_type::object)
    {
      assert(false); // "Unable to add none object."
      return false;
    }
    if (instance == nullptr)
    {
      assert(false); // "Unable to add nullptr object."
      return false;
    }
    // Assets should not be added twice, if this happens it is most likely a programmer error
    if (std::find(begin(objects), end(objects), instance) != end(objects))
    {
      LOG_ERROR("Unable to add asset, it is already registered: {0}", name.c_str());
      return false;
    }
    if (std::find(begin(names), end(names), name) != end(names))
    {
      LOG_ERROR("Unable to add asset, name is already registered. {0}", name.c_str());
      return false;
    }
    instance->set_runtime_id(objects.size());
    objects.push_back(instance);
    names.push_back(name);
    types.push_back(T::get_static_type());
    return true;
  }

  template<derives_from<object> T>
  T* object_registry::get(int id) const
  {
    if (is_valid(id))
    {
      if (get_type(id) == T::get_static_type())
      {
        return static_cast<T*>(objects[id]); // Risky! no RTTI, no dynamic_cast
      }
    }
    return nullptr;
  }

  template<derives_from<object> T>
  T* object_registry::find(const std::string& name)
  {
    for (int i = 0; i < types.size(); i++)
    {
      if (types[i] == T::get_static_type() && names[i] == name)
      {
        assert(objects[i] != nullptr);
        return static_cast<T*>(objects[i]); // Risky! no RTTI, no dynamic_cast
      }
    }
    return nullptr;
  }

  template<derives_from<object> T>
  std::vector<T*> object_registry::get_by_type()
  {
    std::vector<T*> ans;
    for (int i = 0; i < types.size(); i++)
    {
      if (types[i] == T::get_static_type())
      {
        assert(objects[i] != nullptr);
        ans.push_back(static_cast<T*>(objects[i])); // Risky! no RTTI, no dynamic_cast
      }
    }
    return ans;
  }

  template<derives_from<object> T>
  T* object_registry::clone(int source_runtime_id, const std::string& target_name)
  {
    if (!is_valid(source_runtime_id))
    {
      LOG_ERROR("Unable to clone asset: {0} Unknown source runtime id: {1}", target_name.c_str(), source_runtime_id);
      return nullptr;
    }
    if (types[source_runtime_id] != T::get_static_type())
    {
      LOG_ERROR("Unable to clone asset: {0} Type mismatch: {1} and {2}", target_name.c_str(), object_type_names[static_cast<int>(types[source_runtime_id])], object_type_names[static_cast<int>(T::get_static_type())]);
      return nullptr;
    }
    T* source = static_cast<T*>(objects[source_runtime_id]);  // Risky! no RTTI, no dynamic_cast
    if (source == nullptr)
    {
      LOG_ERROR("Unable to clone asset: {0} Invalid source object: {1}", target_name.c_str(), source_runtime_id);
      return nullptr;
    }
    // Shallow copy
    T* obj = T::spawn();
    *obj = *source;
    obj->runtime_id = -1;

    // Register new one
    if (add<T>(obj, target_name))
    {
      return obj;
    }
    return nullptr;
  }


}