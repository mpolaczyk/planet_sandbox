
#include <assert.h>

#include "object/object_registry.h"
#include "engine/log.h"
#include <string>

// Those definitions are moved away from object_registry.cpp to avoid LNK2005 error, when compiling object_registry_inst.cpp
// https://stackoverflow.com/questions/77377405/lnk2005-for-non-templated-function-when-explicitly-instantiating-templated-funct

namespace engine
{
  template<derives_from<object> T>
  bool object_registry::add(T* instance, const std::string& name)
  {
    if (instance == nullptr)
    {
      LOG_ERROR("Unable to add nullptr object.");
      return false;
    }
    /*if (instance->get_class() == object::get_class_static())
    {
      LOG_ERROR("Unable to add object of type object.");
      return false;
    }*/
    if (instance->runtime_id != -1)
    {
      LOG_ERROR("Unable to add already added object.");
      return false;
    }
    // Objects should not be added twice, if this happens it is most likely a programmer error
    if (std::find(begin(objects), end(objects), instance) != end(objects))
    {
      LOG_ERROR("Unable to add object, it is already registered: {0}", name.c_str());
      return false;
    }
    instance->set_runtime_id(objects.size());
    // Make name unique if exists
    std::string final_name = name;
    if (std::find(begin(names), end(names), final_name) != end(names))
    {
      final_name.append(std::to_string(instance->runtime_id));
    }
    objects.push_back(instance);
    names.push_back(name);
    types.push_back(T::get_class_static());
    return true;
  }

  template<derives_from<object> T>
  T* object_registry::get(int id) const
  {
    if (is_valid(id))
    {
      if (objects[id] != nullptr && get_class(id) == T::get_class_static())
      {
        return static_cast<T*>(objects[id]); // Risky! no RTTI, no dynamic_cast
      }
    }
    return nullptr;
  }

  template<derives_from<object> T>
  T* object_registry::find(const std::string& name)
  {
    return const_cast<T*>(find_const<T>(name));
  }

  template<derives_from<object> T>
  const T* object_registry::find_const(const std::string& name)
  {
    for (int i = 0; i < objects.size(); i++)
    {
      if (objects[i] != nullptr && objects[i]->is_class(T::get_class_static()) && names[i] == name) // FIX: Linear search, at some point better structure will be needed
      {
        assert(objects[i] != nullptr);
        return static_cast<T*>(objects[i]); // Risky! no RTTI, no dynamic_cast
      }
    }
    return nullptr;
  }

  template<derives_from<object> T>
  std::vector<T*> object_registry::get_all_by_type()
  {
    std::vector<T*> ans;
    for (int i = 0; i < objects.size(); i++)  // FIX: Linear search, at some point better structure will be needed
    {
      if (objects[i] != nullptr && objects[i]->is_class(T::get_class_static()))
      {
        assert(objects[i] != nullptr);
        ans.push_back(static_cast<T*>(objects[i])); // Risky! no RTTI, no dynamic_cast
      }
    }
    return ans;
  }

  template<derives_from<object> T>
  T* object_registry::copy_shallow(const T* source)
  {
    assert(source != nullptr);

    return copy_shallow<T>(source, source->get_name());
  }

  template<derives_from<object> T>
  T* object_registry::copy_shallow(const T* source, const std::string& name)
  {
    assert(source != nullptr);

    int source_runtime_id = source->get_runtime_id();
    std::string target_name = name;
      
    if (!is_valid(source_runtime_id))
    {
      LOG_ERROR("Unable to clone asset: {0} Unknown source runtime id: {1}", target_name.c_str(), source_runtime_id);
      return nullptr;
    }
    if (types[source_runtime_id] != T::get_class_static())
    {
      LOG_ERROR("Unable to clone asset: {0} Type mismatch: {1} and {2}", target_name.c_str(), types[source_runtime_id]->get_name(), T::get_class_static()->get_name());
      return nullptr;
    }
    // Shallow copy
    T* obj = T::spawn(target_name);
    if (obj == nullptr)
    {
      return nullptr;
    }
    int32_t new_id = obj->runtime_id;
    *obj = *source;
    obj->runtime_id = new_id;
    return obj;
  }

  template<derives_from<object> T>
  T* class_object::spawn_instance(const std::string& name) const
  { 
    return static_cast<T*>(spawn_instance_func(name));
  }

}