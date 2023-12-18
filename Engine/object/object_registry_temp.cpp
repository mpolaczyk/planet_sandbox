
#include <cassert>
#include <algorithm>

#include "object/object_registry.h"
#include "engine/log.h"
#include <string>

// Those definitions are moved away from object_registry.cpp to avoid LNK2005 error, when compiling object_registry_inst.cpp
// https://stackoverflow.com/questions/77377405/lnk2005-for-non-templated-function-when-explicitly-instantiating-templated-funct

namespace engine
{
  template<derives_from<object> T>
  bool object_registry::add(T* instance)
  {
    if (instance == nullptr)
    {
      LOG_ERROR("Unable to add nullptr object.");
      return false;
    }
    if (instance->runtime_id != -1)
    {
      LOG_ERROR("Unable to add already added object.");
      return false;
    }
    // Objects should not be added twice, if this happens it is most likely a programmer error
    if (std::find(begin(objects), end(objects), instance) != end(objects))
    {
      LOG_ERROR("Unable to add object, it is already registered: {0}", instance->runtime_id);
      return false;
    }
    instance->set_runtime_id(static_cast<int>(objects.size()));
    objects.push_back(instance);
    object_classes.push_back(T::get_class_static());
    object_custom_display_names.push_back("");
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
  std::vector<T*> object_registry::get_all_by_type()
  {
    auto type = T::get_class_static();
    return find_all<T>([type](T* obj) -> bool { return obj->is_child_of(type);});
  }

  template<derives_from<object> T>
  T* object_registry::copy_shallow(const T* source)
  {
    assert(source != nullptr);

    int source_runtime_id = source->get_runtime_id();
      
    if (!is_valid(source_runtime_id))
    {
      LOG_ERROR("Unable to clone asset. Unknown source runtime id: {0}", source_runtime_id);
      return nullptr;
    }
    if (object_classes[source_runtime_id] != T::get_class_static())
    {
      LOG_ERROR("Unable to clone asset. Type mismatch: {0} and {1}", object_classes[source_runtime_id]->class_name, T::get_class_static()->class_name);
      return nullptr;
    }
    // Shallow copy
    T* obj = T::spawn();
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
  T* object_registry::spawn_from_class(const class_object* type)
  {
    assert(type);
    return static_cast<T*>(type->spawn_instance_func());
  }

  template<derives_from<object> T>
  T* object_registry::find(std::function<bool(T*)> predicate) const
  {
    const class_object* T_class = T::get_class_static();
    for (int i = 0; i < objects.size(); i++)  // FIX: Linear search, at some point better structure will be needed
    {
      if (objects[i] != nullptr && objects[i]->is_child_of(T_class) && predicate(static_cast<T*>(objects[i])))
      {
        return static_cast<T*>(objects[i]);
      }
    }
    return nullptr;
  }

  template<derives_from<object> T>
  std::vector<T*> object_registry::find_all(std::function<bool(T*)> predicate) const
  {
    std::vector<T*> ans;
    const class_object* T_class = T::get_class_static();
    for (int i = 0; i < objects.size(); i++)  // FIX: Linear search, at some point better structure will be needed
    {
      if (objects[i] != nullptr && objects[i]->is_child_of(T_class) && predicate(static_cast<T*>(objects[i])))
      {
        ans.push_back(static_cast<T*>(objects[i]));
      }
    }
    return ans;
  }
}