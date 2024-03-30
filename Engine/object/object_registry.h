#pragma once

#include <string>
#include <vector>
#include <functional>

#include "core/core.h"
#include "core/concepts.h"
#include "object/object.h"

#define REG fobject_registry::instance()

namespace engine
{
  // Collection of objects of any kind.
  // Registry has ownership on objects.
  // Registry gives runtime ids.
  // Runtime id can't change at runtime.
  // Object destruction is allowed, but no defragmentation will happen
  // No dependence lookup nor reference counting
  // T can be only of type "object" or derived from it
  class ENGINE_API fobject_registry final
  {
  public:
    fobject_registry() = default;
    ~fobject_registry();
    fobject_registry(const fobject_registry&) = delete;
    fobject_registry(fobject_registry&&) = delete;
    fobject_registry& operator=(fobject_registry&&) = delete;
    fobject_registry& operator=(const fobject_registry&) = delete;

    static fobject_registry& instance()
    {
      static fobject_registry singleton;
      return singleton;
    }
    
    bool is_valid(int id) const;
    std::string get_custom_display_name(int id) const;
    void set_custom_display_name(int id, const std::string& name);
    const oclass_object* get_class(int id) const;
    void destroy(int id); 
    std::vector<oobject*> get_all(bool no_nullptr = true);
    std::vector<int> get_all_ids(const oclass_object* type, bool no_nullptr = true) const;

    const oclass_object* find_class(const std::string& name) const;
    std::vector<const oclass_object*> get_classes() const;
    const oclass_object* register_class(const std::string& class_name, const std::string& parent_class_name, spawn_instance_func_type spawn_func);
    void create_class_objects();

  protected:
    // Index is runtime id.
    std::vector<oobject*> objects;                           // Main object registry. Holds the ownership.
    std::vector<const oclass_object*> object_classes;        // Does not store class objects but points to an instance owned by the objects vector (that happens to be the class_object)
    std::vector<std::string> object_custom_display_names;   // Don't store this on instances, don't break the alignment, cache them here.

    // Index is not related to the objects vector
    std::vector<const oclass_object*> class_objects;     // A subset of objects of the class_object type. No ownership.    

  public:

    template<derives_from<oobject> T >
    bool add(T* instance);

    template<derives_from<oobject> T>
    T* get(int id) const;

    template<derives_from<oobject> T>
    std::vector<T*> get_all_by_type();

    template<derives_from<oobject> T>
    T* copy_shallow(const T* source);

    template<derives_from<oobject> T>
    T* find(std::function<bool(T*)> predicate) const;

    template<derives_from<oobject> T>
    std::vector<T*> find_all(std::function<bool(T*)> predicate) const;

    template<derives_from<oobject> T>
    T* spawn_from_class(const oclass_object* type);
  };
}