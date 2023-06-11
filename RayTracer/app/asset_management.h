#pragma once

#include <map>

#include "app/factories.h"

template<typename T>
class asset_instances
{
public:
  bool is_id_in_use(const std::string& id) const;
  bool try_add(T* instance);
  void remove(const std::string& id);
  T* get(const std::string& id) const;
  std::vector<std::string> get_ids() const;
  std::vector<std::string> get_names() const;
  int get_index_by_name(const std::string& name) const;
  int get_index_by_id(const std::string& id) const;

  std::map<std::string, T*> registry;
};