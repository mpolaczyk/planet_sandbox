#pragma once

#include <string>
#include <vector>
#include <functional>

#include "core/core.h"
#include "core/concepts.h"
#include "object/object.h"

#define REG object_registry::instance()

namespace engine
{
  // Collection of objects of any kind.
  // Registry has ownership on objects.
  // Registry gives runtime ids.
  // Runtime id can't change at runtime.
  // Object destruction is allowed, but no defragmentation will happen
  // No dependence lookup nor reference counting
  // T can be only of type "object" or derived from it
  class ENGINE_API object_registry final
  {
  public:
    object_registry() = default;
    ~object_registry();
    object_registry(const object_registry&) = delete;
    object_registry(object_registry&&) = delete;
    object_registry& operator=(object_registry&&) = delete;
    object_registry& operator=(const object_registry&) = delete;

    static object_registry& instance()
    {
      static object_registry singleton;
      return singleton;
    }
    
    bool is_valid(int id) const;
    std::string get_custom_display_name(int id) const;
    void set_custom_display_name(int id, const std::string& name);
    const class_object* get_class(int id) const;
    void destroy(int id); 
    std::vector<object*> get_all(bool no_nullptr = true);   // FIX those getters are stupid, add iterators to objects, names and class objects
    std::vector<int> get_all_ids(const class_object* type, bool no_nullptr = true) const;

    const class_object* find_class(const std::string& name) const;
    std::vector<const class_object*> get_classes() const;
    const class_object* register_class(const std::string& class_name, const std::string& parent_class_name, spawn_instance_func_type spawn_func);
    void create_class_objects();

  protected:
    // Index is runtime id.
    std::vector<object*> objects;                       // Main object registry. Holds the ownership.
    std::vector<const class_object*> types;             // Does not store class objects but points to an instance owned by the objects vector (that happens to be the class_object)
    std::vector<std::string> custom_display_names;      // Don't store this on instances, don't break the alignment, cache them here.

    // Index is not related to the objects vector
    std::vector<const class_object*> class_objects;     // A subset of objects of the class_object type. No ownership. FIX Classes can't be deleted.    

  public:

    template<derives_from<object> T >
    bool add(T* instance);   // FIX private?

    template<derives_from<object> T>
    T* get(int id) const;

    template<derives_from<object> T>
    std::vector<T*> get_all_by_type();

    template<derives_from<object> T>
    T* copy_shallow(const T* source);

    template<derives_from<object> T>
    T* find(std::function<bool(const T*)> predicate) const;
  };
}