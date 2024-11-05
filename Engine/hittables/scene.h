#pragma once

#include <string>
#include <vector>
#include <DirectXMath.h>

#include "hittables.h"
#include "math/camera.h"
#include "renderer/scene_acceleration.h"

namespace engine
{
  class hlight;
  class rrenderer_base;

  class ENGINE_API hscene : public hhittable_base
  {
  public:
    OBJECT_DECLARE(hscene, hittable)
    OBJECT_DECLARE_VISITOR

    virtual uint32_t get_hash() const override;
    virtual void load_resources() override;

    void add(hhittable_base* object);
    void remove(int object_id);

    std::vector<const hlight*> query_lights() const;

    // Persistent members
    std::vector<hhittable_base*> objects;
    rrenderer_base* renderer = nullptr;     // TODO remove it from here, should be application setting
    DirectX::XMFLOAT4 ambient_light_color;  // TODO move to renderer
    DirectX::XMVECTORF32 clear_color;       // TODO move to renderer
    fcamera camera_config;
    fscene_acceleration scene_acceleration;
  };
}
