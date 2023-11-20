
#include "persistence/camera_config_json.h"

#include "nlohmann/json.hpp"
#include "math/camera.h"
#include "persistence/vec3_json.h"

namespace engine
{
  nlohmann::json camera_config_serializer::serialize(const camera_config& value)
  {
    nlohmann::json j;
    j["field_of_view"] = value.field_of_view;
    j["aspect_ratio_h"] = value.aspect_ratio_h;
    j["aspect_ratio_w"] = value.aspect_ratio_w;
    j["aperture"] = value.aperture;
    j["dist_to_focus"] = value.dist_to_focus;
    j["type"] = value.type;
    j["look_from"] = vec3_serializer::serialize(value.look_from);
    j["look_dir"] = vec3_serializer::serialize(value.look_dir);
    return j;
  }

  camera_config camera_config_serializer::deserialize(const nlohmann::json& j)
  {
    camera_config value;
    TRY_PARSE(float, j, "field_of_view", value.field_of_view);
    TRY_PARSE(float, j, "aspect_ratio_h", value.aspect_ratio_h);
    TRY_PARSE(float, j, "aspect_ratio_w", value.aspect_ratio_w);
    TRY_PARSE(float, j, "aperture", value.aperture);
    TRY_PARSE(float, j, "dist_to_focus", value.dist_to_focus);
    TRY_PARSE(float, j, "type", value.type);

    nlohmann::json jlook_dir;
    if (TRY_PARSE(nlohmann::json, j, "look_dir", jlook_dir)) { value.look_dir = vec3_serializer::deserialize(jlook_dir); }

    nlohmann::json jlook_from;
    if (TRY_PARSE(nlohmann::json, j, "look_from", jlook_from)) { value.look_from = vec3_serializer::deserialize(jlook_from); }

    return value;
  }
}