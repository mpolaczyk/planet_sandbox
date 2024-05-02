#pragma once

#include "nlohmann/json_fwd.hpp"
#include "core/core.h"

namespace DirectX
{
  struct XMFLOAT3;
  struct XMFLOAT4;
  struct XMVECTORF32;
}

namespace engine
{
  class frenderer_config;
  class fcamera;
  struct fsoft_asset_ptr_base;
  struct fvec3;
  struct flight_properties;
  struct fmaterial_properties;
  
  class ENGINE_API fpersistence
  {
  public:
    
    static nlohmann::json serialize(const fsoft_asset_ptr_base& value);
    static void deserialize(const nlohmann::json& j, fsoft_asset_ptr_base& out_value);

    static nlohmann::json serialize(const fvec3& value);
    static void deserialize(const nlohmann::json& j, fvec3& out_value);

    static nlohmann::json serialize(const DirectX::XMFLOAT3& value);
    static void deserialize(const nlohmann::json& j, DirectX::XMFLOAT3& out_value);

    static nlohmann::json serialize(const DirectX::XMFLOAT4& value);
    static void deserialize(const nlohmann::json& j, DirectX::XMFLOAT4& out_value);

    static nlohmann::json serialize(const DirectX::XMVECTORF32& value);
    static void deserialize(const nlohmann::json& j, DirectX::XMVECTORF32& out_value);
    
    static nlohmann::json serialize(const fcamera& value);
    static void deserialize(const nlohmann::json& j, fcamera& out_value);

    static nlohmann::json serialize(const frenderer_config& value);
    static void deserialize(const nlohmann::json& j, frenderer_config& out_value);

    static nlohmann::json serialize(const flight_properties& value);
    static void deserialize(const nlohmann::json& j, flight_properties& out_value);

    static nlohmann::json serialize(const fmaterial_properties& value);
    static void deserialize(const nlohmann::json& j, fmaterial_properties& out_value);
    
  };
}