#pragma once

#include <string>
#include <vector>

#include "core/core.h"
#include "core/concepts.h"
#include "object/object.h"

namespace engine
{
  // Collection of objects of any kind.
  // Registry has ownership on objects.
  // Registry gives runtime ids.
  // Runtime id can't change at runtime.
  // Object destruction is allowed, but no defragmentation will happen
  // No dependence lookup nor reference counting
  // T can be only of type "object" or derived from it
  class ENGINE_API object_registry
  {
  public:
    // No copy, no move
    object_registry() = default;
    ~object_registry();
    object_registry(const object_registry&) = delete;
    object_registry& operator=(const object_registry&) = delete;

    bool is_valid(int id) const;
    std::string get_name(int id) const;
    const class_object* get_class(int id) const;
    void destroy(int id);
    std::vector<object*> get_all(bool no_nullptr = true);
    std::vector<int> get_all_ids(const class_object* type, bool no_nullptr = true) const;
    std::vector<std::string> get_all_names(const class_object* type, bool no_nullptr = true) const;

    const class_object* find_class(const std::string& name);
    void register_class(class_object* instance);
    void create_class_objects();

  protected:
    // Runtime id is an index
    std::vector < const class_object*> class_objects;
    std::vector<object*> objects;
    std::vector<const class_object*> types; // Does not store class objects but points to an instance owned by the objects vector (that happens to be the class_object)
    std::vector<std::string> names;         // FIX Do we need this? can we use methods on the type?

  public:

    template<derives_from<object> T >
    bool add(T* instance, const std::string& name);   // FIX private?

    template<derives_from<object> T>
    T* get(int id) const;

    template<derives_from<object> T>
    T* find(const std::string& name);

    template<derives_from<object> T>
    const T* find_const(const std::string& name);


    template<derives_from<object> T>
    std::vector<T*> get_all_by_type();

    template<derives_from<object> T>
    T* copy_shallow(const T* source);

    template<derives_from<object> T>
    T* copy_shallow(const T* source, const std::string& name);
  };


  // Global singleton
  extern object_registry* g_object_registy;
  ENGINE_API object_registry* get_object_registry();
}