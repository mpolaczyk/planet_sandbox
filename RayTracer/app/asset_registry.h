#pragma once

#include <map>
#include <string>


template<typename K, typename V>
concept has_get_id = requires(const V v) {
  { v.get_id() } -> std::same_as<K>;
};

template<typename K, typename V>
concept has_get_name = requires(const V v) {
  { v.get_name() } -> std::convertible_to<std::string>;
};

template<typename K, typename V>
class asset_registry
{
  // No base class and inheritance for assets. Only compile time requirements.
  // The same can be done with c++20 requires clause, but forward declaration gets complicated.
  static_assert(has_get_id<K, V>);
  static_assert(has_get_name<K, V>);

public:
  bool is_id_in_use(const K& id) const;
  bool try_add(V* instance);
  void remove(const K& id);
  V* get(const K& id) const;
  std::vector<K> get_ids() const;
  std::vector<std::string> get_names() const;
  int get_index_by_name(const std::string& name) const;
  int get_index_by_id(const K& id) const;

  std::map<K, V*> registry;
};

