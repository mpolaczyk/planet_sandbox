#pragma once

#include <string>

#include "core/core.h"

#include "asset/asset.h"
#include "math/vec3.h"
#include "asset/soft_asset_ptr.h"

namespace engine
{
  enum class material_type
  {
    none = 0,
    universal,
    light
  };
  static inline const char* material_type_names[] =
  {
    "None",
    "Universal",
    "Light"
  };
  enum class renderer_type
  {
    example = 0,
    preview,
    preview_normals,
    preview_faces,
    reference,
    ispc
  };
  static inline const char* renderer_type_names[] =
  {
    "CPU Example",
    "CPU Preview",
    "CPU Preview Normals",
    "CPU Preview Faces",
    "CPU Reference",
    "CPU ISPC (Example only)"
  };

  enum class hittable_type
  {
    scene = 0,
    sphere,
    xy_rect,
    xz_rect,
    yz_rect,
    static_mesh
  };
  static inline const char* hittable_type_names[] =
  {
    "Scene",
    "Sphere",
    "XY Rectangle",
    "XZ Rectangle",
    "YZ Rectangle",
    "Static Mesh"
  };

  class ENGINE_API material : public asset
  {
  public:
    static asset_type get_static_asset_type();
    static material* load(const std::string& material_name);
    static void save(material* object);
    static material* spawn();

    virtual std::string get_display_name() const override;

    material() = default;
    explicit material(material_type type) : type(type)
    {
      if (type == material_type::light)
      {
        emitted_color = vec3(1.0f, 1.0f, 1.0f);
      }
    }
    material(std::string&& id, material_type type) : material(type)
    {
      id = std::move(id);
    }

    // JSON persistent
    material_type type = material_type::none;
    vec3 color;
    vec3 emitted_color;
    vec3 gloss_color;
    vec3 pad;
    float smoothness = 0.0f;
    float gloss_probability = 0.0f;
    float refraction_probability = 0.0f;
    float refraction_index = 1.0f;

    //void draw_edit_panel(); FIX
  };
}
