#pragma once

#include <string>
#include <vector>

#include "core/core.h"
#include "asset/asset.h"

namespace engine
{
  // Collection of objects of any kind.
  // Registry has ownership on objects.
  // Registry gives runtime ids.
  // The fact that object is in the registry should mean it has resources loaded.
  // For now I assume there is no way to remove an object -> no defragmentation
  // Objects can't be deleted from memory -> no dependence lookup or reference counting
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
    std::vector<object*> get_all();
    std::vector<int> get_all_ids(object_type type) const;
    std::vector<std::string> get_all_names(object_type type) const;

  protected:
    // Runtime id is an index
    // None of this can change at runtime after is added
    std::vector<std::string> names;
    std::vector<object_type> types;
    std::vector<object*> objects;

  public:

    template<typename T>
    bool add(T* object, const std::string& name);

    template<typename T>
    T* get(int id) const;

    template<typename T>
    T* find(const std::string& name);

    template<typename T>
    std::vector<T*> get_by_type();

    template<typename T>
    T* clone(int source_runtime_id, const std::string& target_name);
  };


  // Singleton
  ENGINE_API object_registry* get_object_registry();
}