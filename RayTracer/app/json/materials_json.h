#pragma once

#include "math/materials.h"
#include "app/asset_management.h"

#include "app/json/serializable.h"

class material_serializer
{
public:
  static nlohmann::json serialize(const material* value);
  static void deserialize(const nlohmann::json& j, material* out_value);
};

class material_instances_serializer
{
public:
  static nlohmann::json serialize(const material_instances* value);
  static void deserialize(const nlohmann::json&, material_instances* out_value);
};
