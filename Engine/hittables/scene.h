#pragma once

#include <string>
#include <vector>
#include <array>

#include "hittables.h"


#include "math/vec3.h"
#include "math/ray.h"
#include "math/hit.h"
#include "math/aabb.h"

namespace engine
{
  class ENGINE_API scene : public hittable
  {
  public:
    OBJECT_DECLARE(scene, hittable)

    virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
    virtual bool get_bounding_box(aabb& out_box) const override;
    virtual vec3 get_origin() const override { assert(false); return vec3(0, 0, 0); };
    virtual vec3 get_extent() const override { assert(false); return vec3(0, 0, 0); };
    virtual void set_origin(const vec3& value) override { };
    virtual void set_extent(float value) override { assert(false); };

    virtual uint32_t get_hash() const override;
    virtual scene* clone() const override;
    virtual void load_resources() override;
    virtual void pre_render() override;

    void add(hittable* object);
    void remove(int object_id);

    void build_boxes();
    void update_materials();
    void query_lights();
    hittable* get_random_light();

    // Persistent members
    std::vector<hittable*> objects;

    // Runtime members
    int32_t lights_num = 0;
    std::array<hittable*, MAX_LIGHTS> lights;
  };
}