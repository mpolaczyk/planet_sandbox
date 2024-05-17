#include "persistence/persistence.h"
#include "persistence/persistence_helper.h"

#include "asset/soft_asset_ptr.h"
#include "math/camera.h"

#include "nlohmann/json.hpp"
#include "object/object_registry.h"

#include <DirectXMath.h>

#include "renderers/aligned_structs.h"


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

  nlohmann::json fpersistence::serialize(const DirectX::XMFLOAT4& value)
  {
    nlohmann::json j;
    j["x"] = value.x;
    j["y"] = value.y;
    j["z"] = value.z;
    j["w"] = value.w;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, DirectX::XMFLOAT4& out_value)
  {
    TRY_PARSE(float, j, "x", out_value.x);
    TRY_PARSE(float, j, "y", out_value.y);
    TRY_PARSE(float, j, "z", out_value.z);
    TRY_PARSE(float, j, "w", out_value.w);
  }

  nlohmann::json fpersistence::serialize(const DirectX::XMVECTORF32& value)
  {
    nlohmann::json j;
    j["x"] = value.f[0];
    j["y"] = value.f[1];
    j["z"] = value.f[2];
    j["w"] = value.f[3];
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, DirectX::XMVECTORF32& out_value)
  {
    TRY_PARSE(float, j, "x", out_value.f[0]);
    TRY_PARSE(float, j, "y", out_value.f[1]);
    TRY_PARSE(float, j, "z", out_value.f[2]);
    TRY_PARSE(float, j, "w", out_value.f[3]);
  }

  nlohmann::json fpersistence::serialize(const fsoft_asset_ptr_base& value)
  {
    nlohmann::json j;
    j["name"] = value.name;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, fsoft_asset_ptr_base& out_value)
  {
    TRY_PARSE(std::string, j, "name", out_value.name);
  }

  nlohmann::json fpersistence::serialize(const fcamera& value)
  {
    nlohmann::json j;
    j["field_of_view"] = value.field_of_view;
    j["look_from"] = fpersistence::serialize(value.location);
    j["pitch"] = value.pitch;
    j["yaw"] = value.yaw;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, fcamera& out_value)
  {
    TRY_PARSE(float, j, "field_of_view", out_value.field_of_view);
    TRY_PARSE(float, j, "pitch", out_value.pitch);
    TRY_PARSE(float, j, "yaw", out_value.yaw);

    nlohmann::json jlook_from;
    if(TRY_PARSE(nlohmann::json, j, "look_from", jlook_from)) { fpersistence::deserialize(jlook_from, out_value.location); }
  }

  nlohmann::json fpersistence::serialize(const flight_properties& value)
  {
    nlohmann::json j;
    j["direction"] = fpersistence::serialize(value.direction);
    j["color"] = fpersistence::serialize(value.color);
    j["spot_angle"] = value.spot_angle;
    j["constant_attenuation"] = value.constant_attenuation;
    j["linear_attenuation"] = value.linear_attenuation;
    j["quadratic_attenuation"] = value.quadratic_attenuation;
    j["light_type"] = value.light_type;
    j["enabled"] = value.enabled;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, flight_properties& out_value)
  {
    nlohmann::json jdirection;
    if(TRY_PARSE(nlohmann::json, j, "direction", jdirection)) { fpersistence::deserialize(jdirection, out_value.direction); }

    nlohmann::json jcolor;
    if(TRY_PARSE(nlohmann::json, j, "color", jcolor)) { fpersistence::deserialize(jcolor, out_value.color); }

    TRY_PARSE(float, j, "spot_angle", out_value.spot_angle);
    TRY_PARSE(float, j, "constant_attenuation", out_value.constant_attenuation);
    TRY_PARSE(float, j, "linear_attenuation", out_value.linear_attenuation);
    TRY_PARSE(float, j, "quadratic_attenuation", out_value.quadratic_attenuation);
    TRY_PARSE(int, j, "light_type", out_value.light_type);
    TRY_PARSE(int, j, "enabled", out_value.enabled);
  }

  nlohmann::json fpersistence::serialize(const fmaterial_properties& value)
  {
    nlohmann::json j;
    j["emissive"] = fpersistence::serialize(value.emissive);
    j["ambient"] = fpersistence::serialize(value.ambient);
    j["diffuse"] = fpersistence::serialize(value.diffuse);
    j["specular"] = fpersistence::serialize(value.specular);
    j["specular_power"] = value.specular_power;
    j["use_texture"] = value.use_texture;
    return j;
  }

  void fpersistence::deserialize(const nlohmann::json& j, fmaterial_properties& out_value)
  {
    nlohmann::json jemissive;
    if(TRY_PARSE(nlohmann::json, j, "emissive", jemissive)) { fpersistence::deserialize(jemissive, out_value.emissive); }
    nlohmann::json jambient;
    if(TRY_PARSE(nlohmann::json, j, "ambient", jambient)) { fpersistence::deserialize(jambient, out_value.ambient); }
    nlohmann::json jdiffuse;
    if(TRY_PARSE(nlohmann::json, j, "diffuse", jdiffuse)) { fpersistence::deserialize(jdiffuse, out_value.diffuse); }
    nlohmann::json jspecular;
    if(TRY_PARSE(nlohmann::json, j, "specular", jspecular)) { fpersistence::deserialize(jspecular, out_value.specular); }
    TRY_PARSE(float, j, "specular_power", out_value.specular_power);
    TRY_PARSE(int, j, "use_texture", out_value.use_texture);
  }
}