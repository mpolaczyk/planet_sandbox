#pragma once

#include <string>
#include <vector>

#include <DirectXMath.h>

#include "engine/hittable.h"
#include "engine/math/camera.h"
#include "engine/renderer/scene_acceleration.h"

namespace engine
{
  class hlight;
  class rrenderer_base;

  class ENGINE_API hscene : public hhittable_base
  {
  public:
    OBJECT_DECLARE(hscene, hittable)
    OBJECT_DECLARE_VISITOR

    CTOR_DEFAULT(hscene)
    CTOR_COPY_DEFAULT(hscene)
    CTOR_MOVE_DELETE(hscene)
    virtual ~hscene() override;
    
    virtual uint32_t get_hash() const override;
    virtual void load_resources() override;
    void create_scene_physics_state();
    void update_scene_physics_state();
    void save_pre_physics_scene_state();
    void restore_pre_physics_scene_state();
    void destroy_scene_physics_state();

    void add(hhittable_base* object);
    void remove(int object_id);

    std::vector<const hlight*> query_lights() const;

    // Persistent members
    std::vector<hhittable_base*> objects;
    rrenderer_base* renderer = nullptr;   // No need for shared ptr, managed object
    DirectX::XMFLOAT4 ambient_light_color;
    fcamera camera;

    // Runtime members
    fscene_acceleration scene_acceleration;
  };
}
