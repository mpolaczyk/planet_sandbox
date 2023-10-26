#pragma once

#include "app/json/serializable.h"

#include "engine.h"

struct soft_asset_ptr_base;

class material_serializer
{
public:
  static nlohmann::json serialize(const engine::material* value);
  static void deserialize(const nlohmann::json& j, engine::material* out_value);
};

class mesh_serializer
{
public:
  static nlohmann::json serialize(const engine::mesh* value);
  static void deserialize(const nlohmann::json& j, engine::mesh* out_value);
};

class texture_serializer
{
public:
  static nlohmann::json serialize(const engine::texture* value);
  static void deserialize(const nlohmann::json& j, engine::texture* out_value);
};

class soft_asset_ptr_base_serializer
{
public:
  static nlohmann::json serialize(const soft_asset_ptr_base* value);
  static void deserialize(const nlohmann::json& j, soft_asset_ptr_base* out_value);
};