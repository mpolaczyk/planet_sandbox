#pragma once

#include <string>
#include <vector>
#include <memory>

#include <DirectXMath.h>

#include "hittables.h"
#include "math/camera.h"
#include "renderer/scene_acceleration.h"

namespace reactphysics3d
{
  class PhysicsWorld;
}

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
    void create_physics_state();

    void add(hhittable_base* object);
    void remove(int object_id);

    std::vector<const hlight*> query_lights() const;

    // Persistent members
    std::vector<hhittable_base*> objects;
    rrenderer_base* renderer = nullptr;
    DirectX::XMFLOAT4 ambient_light_color;
    fcamera camera_config;

    // Runtime members
    fscene_acceleration scene_acceleration;
    reactphysics3d::PhysicsWorld* physics_world;
  };
}
