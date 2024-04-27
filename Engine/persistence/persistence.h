#pragma once

#include "nlohmann/json_fwd.hpp"
#include "core/core.h"

namespace DirectX
{
  struct XMFLOAT3;  
}

namespace engine
{
  class frenderer_config;
  class fcamera_config;
  struct fsoft_asset_ptr_base;
  struct fvec3;
  
  class ENGINE_API fpersistence
  {
  public:
    
    static nlohmann::json serialize(const fsoft_asset_ptr_base& value);
    static void deserialize(const nlohmann::json& j, fsoft_asset_ptr_base& out_value);

    static nlohmann::json serialize(const fvec3& value);
    static void deserialize(const nlohmann::json& j, fvec3& out_value);

    static nlohmann::json serialize(const DirectX::XMFLOAT3& value);
    static void deserialize(const nlohmann::json& j, DirectX::XMFLOAT3& out_value);

    static nlohmann::json serialize(const fcamera_config& value);
    static void deserialize(const nlohmann::json& j, fcamera_config& out_value);

    static nlohmann::json serialize(const frenderer_config& value);
    static void deserialize(const nlohmann::json& j, frenderer_config& out_value);
    
  };
}