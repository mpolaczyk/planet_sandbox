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
    object_type get_type(int id) const;
    void destroy(int id);
    std::vector<object*> get_all(bool no_nullptr = true);
    std::vector<int> get_all_ids(object_type type, bool no_nullptr = true) const;
    std::vector<std::string> get_all_names(object_type type, bool no_nullptr = true) const;

  protected:
    // Runtime id is an index
    std::vector<std::string> names;
    std::vector<object_type> types;
    std::vector<object*> objects;

  public:

    template<derives_from<object> T >
    bool add(T* instance, const std::string& name);

    template<derives_from<object> T>
    T* get(int id) const;

    template<derives_from<object> T>
    T* find(const std::string& name);

    template<derives_from<object> T>
    std::vector<T*> get_all_by_type();  // FIX does not have to be templated, object_type is enough

    template<derives_from<object> T>
    T* clone(int source_runtime_id, const std::string& target_name);
  };


  // Singleton
  ENGINE_API object_registry* get_object_registry();
}