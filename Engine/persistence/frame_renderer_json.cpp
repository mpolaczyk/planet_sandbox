#include "persistence/frame_renderer_json.h"
#include "nlohmann/json.hpp"
#include "renderer/cpu_renderer_base.h"


#include "object/object_registry.h"

namespace engine
{
  nlohmann::json renderer_config_serializer::serialize(const renderer_config& value)
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

  renderer_config renderer_config_serializer::deserialize(const nlohmann::json& j)
  {
    renderer_config value;
    TRY_PARSE(int, j, "rays_per_pixel", value.rays_per_pixel);
    TRY_PARSE(int, j, "ray_bounces", value.ray_bounces);

    std::string type_name;
    TRY_PARSE(std::string, j, "type", type_name);
    value.type = get_object_registry()->find_class(type_name);

    //assert(cpu_renderer_base::is_parent_of_static(value.type)); // FIX
    TRY_PARSE(bool, j, "reuse_buffer", value.reuse_buffer);
    TRY_PARSE(int, j, "resolution_vertical", value.resolution_vertical);
    TRY_PARSE(int, j, "resolution_horizontal", value.resolution_horizontal);
    TRY_PARSE(float, j, "white_point", value.white_point);
    return value;
  }
}