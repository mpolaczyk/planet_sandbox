#include "persistence/persistence.h"
#include "persistence/persistence_helper.h"

#include "asset/soft_asset_ptr.h"
#include "math/camera.h"
#include "renderer/cpu_renderer_base.h"

#include "nlohmann/json.hpp"
#include "object/object_registry.h"

namespace engine
{
  nlohmann::json persistence::serialize(const vec3& value)
  {
    nlohmann::json j;
    j["x"] = value.x;
    j["y"] = value.y;
    j["z"] = value.z;
    return j;
  }
  void persistence::deserialize(const nlohmann::json& j, vec3& out_value)
  {
    TRY_PARSE(float, j, "x", out_value.x);
    TRY_PARSE(float, j, "y", out_value.y);
    TRY_PARSE(float, j, "z", out_value.z);
  }

  nlohmann::json persistence::serialize(const soft_asset_ptr_base& value)
  {
    nlohmann::json j;
    j["name"] =value.name;
    return j;
  }
  void persistence::deserialize(const nlohmann::json& j, soft_asset_ptr_base& out_value)
  {
    TRY_PARSE(std::string, j, "name", out_value.name);
  }

  nlohmann::json persistence::serialize(const camera_config& value)
  {
    nlohmann::json j;
    j["field_of_view"] = value.field_of_view;
    j["aspect_ratio_h"] = value.aspect_ratio_h;
    j["aspect_ratio_w"] = value.aspect_ratio_w;
    j["aperture"] = value.aperture;
    j["dist_to_focus"] = value.dist_to_focus;
    j["type"] = value.type;
    j["look_from"] = persistence::serialize(value.look_from);
    j["look_dir"] = persistence::serialize(value.look_dir);
    return j;
  }
  void persistence::deserialize(const nlohmann::json& j, camera_config& out_value)
  {
    TRY_PARSE(float, j, "field_of_view", out_value.field_of_view);
    TRY_PARSE(float, j, "aspect_ratio_h", out_value.aspect_ratio_h);
    TRY_PARSE(float, j, "aspect_ratio_w", out_value.aspect_ratio_w);
    TRY_PARSE(float, j, "aperture", out_value.aperture);
    TRY_PARSE(float, j, "dist_to_focus", out_value.dist_to_focus);
    TRY_PARSE(float, j, "type", out_value.type);

    nlohmann::json jlook_dir;
    if (TRY_PARSE(nlohmann::json, j, "look_dir", jlook_dir)) { persistence::deserialize(jlook_dir, out_value.look_dir); }

    nlohmann::json jlook_from;
    if (TRY_PARSE(nlohmann::json, j, "look_from", jlook_from)) { persistence::deserialize(jlook_from, out_value.look_from); }
  }

  nlohmann::json persistence::serialize(const renderer_config& value)
  {
    nlohmann::json j;
    j["rays_per_pixel"] = value.rays_per_pixel;
    j["ray_bounces"] = value.ray_bounces;
    j["type"] = value.type->class_name;
    j["reuse_buffer"] = value.reuse_buffer;
    j["resolution_vertical"] = value.resolution_vertical;
    j["resolution_horizontal"] = value.resolution_horizontal;
    j["white_point"] = value.white_point;
    return j;
  }

  void persistence::deserialize(const nlohmann::json& j, renderer_config& out_value)
  {
    TRY_PARSE(int, j, "rays_per_pixel", out_value.rays_per_pixel);
    TRY_PARSE(int, j, "ray_bounces", out_value.ray_bounces);

    std::string type_name;
    TRY_PARSE(std::string, j, "type", type_name);
    out_value.type = REG.find_class(type_name);

    //assert(cpu_renderer_base::is_parent_of_static(value.type)); // FIX
    TRY_PARSE(bool, j, "reuse_buffer", out_value.reuse_buffer);
    TRY_PARSE(int, j, "resolution_vertical", out_value.resolution_vertical);
    TRY_PARSE(int, j, "resolution_horizontal", out_value.resolution_horizontal);
    TRY_PARSE(float, j, "white_point", out_value.white_point);
  }
}
