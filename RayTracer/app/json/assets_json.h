#pragma once

#include "app/json/serializable.h"

class material;
class mesh;
struct soft_asset_ptr_base;

class material_serializer
{
public:
  static nlohmann::json serialize(const material* value);
  static void deserialize(const nlohmann::json& j, material* out_value);
};

class mesh_serializer
{
public:
  static nlohmann::json serialize(const mesh* value);
  static void deserialize(const nlohmann::json& j, mesh* out_value);
};

class soft_asset_ptr_base_serializer
{
public:
  static nlohmann::json serialize(const soft_asset_ptr_base* value);
  static void deserialize(const nlohmann::json& j, soft_asset_ptr_base* out_value);
};