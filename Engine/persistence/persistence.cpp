#include "persistence/persistence.h"
#include "persistence/persistence_helper.h"

#include "asset/soft_asset_ptr.h"
#include "math/camera.h"
#include "renderer/renderer_base.h"

#include "nlohmann/json.hpp"
#include "object/object_registry.h"

#include <DirectXMath.h>


namespace engine
{
  nlohmann::json fpersistence::serialize(const fvec3& value)
  {
    nlohmann::json j;
    j["x"] = value.x;
    j["y"] = value.y;
    j["z"] = value.z;
    return j;
  }
  void fpersistence::deserialize(const nlohmann::json& j, fvec3& out_value)
  {
    TRY_PARSE(float, j, "x", out_value.x);
    TRY_PARSE(float, j, "y", out_value.y);
    TRY_PARSE(float, j, "z", out_value.z);
  }

  nlohmann::json fpersistence::serialize(const DirectX::XMFLOAT3& value)
  {
    nlohmann::json j;
    j["x"] = value.x;
    j["y"] = value.y;
    j["z"] = value.z;
    return j;
  }
  void fpersistence::deserialize(const nlohmann::json& j, DirectX::XMFLOAT3& out_value)
  {
    TRY_PARSE(float, j, "x", out_value.x);
    TRY_PARSE(float, j, "y", out_value.y);
    TRY_PARSE(float, j, "z", out_value.z);
  }

  nlohmann::json fpersistence::serialize(const fsoft_asset_ptr_base& value)
  {
    nlohmann::json j;
    j["name"] =value.name;
    return j;
  }
  void fpersistence::deserialize(const nlohmann::json& j, fsoft_asset_ptr_base& out_value)
  {
    TRY_PARSE(std::string, j, "name", out_value.name);
  }

  nlohmann::json fpersistence::serialize(const fcamera_config& value)
  {
    nlohmann::json j;
    j["field_of_view"] = value.field_of_view;
    j["aspect_ratio_h"] = value.aspect_ratio_h;
    j["aspect_ratio_w"] = value.aspect_ratio_w;
    j["look_from"] = fpersistence::serialize(value.location);
    j["pitch"] = value.pitch;
    j["yaw"] = value.yaw;
    return j;
  }
  void fpersistence::deserialize(const nlohmann::json& j, fcamera_config& out_value)
  {
    TRY_PARSE(float, j, "field_of_view", out_value.field_of_view);
    TRY_PARSE(float, j, "aspect_ratio_h", out_value.aspect_ratio_h);
    TRY_PARSE(float, j, "aspect_ratio_w", out_value.aspect_ratio_w);
    TRY_PARSE(float, j, "pitch", out_value.pitch);
    TRY_PARSE(float, j, "yaw", out_value.yaw);
    
    nlohmann::json jlook_from;
    if (TRY_PARSE(nlohmann::json, j, "look_from", jlook_from)) { fpersistence::deserialize(jlook_from, out_value.location); }
  }

  nlohmann::json fpersistence::serialize(const frenderer_config& value)
  {
    nlohmann::json j;
    j["type"] = value.type->get_class_name();
    j["resolution_vertical"] = value.resolution_vertical;
    j["resolution_horizontal"] = value.resolution_horizontal;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, frenderer_config& out_value)
  {
    std::string type_name;
    TRY_PARSE(std::string, j, "type", type_name);
    out_value.type = REG.find_class(type_name);
    assert(out_value.type);
    
    TRY_PARSE(int, j, "resolution_vertical", out_value.resolution_vertical);
    TRY_PARSE(int, j, "resolution_horizontal", out_value.resolution_horizontal);
  }
}
