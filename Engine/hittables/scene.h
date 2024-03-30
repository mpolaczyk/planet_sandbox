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
  class ENGINE_API hscene : public hhittable_base
  {
  public:
    OBJECT_DECLARE(hscene, hittable)
    OBJECT_DECLARE_VISITOR
    
    virtual bool hit(const fray& in_ray, float t_max, fhit_record& out_hit) const override;
    virtual bool get_bounding_box(faabb& out_box) const override;
    virtual fvec3 get_origin() const override { assert(false); return fvec3(0, 0, 0); };
    virtual fvec3 get_extent() const override { assert(false); return fvec3(0, 0, 0); };
    virtual void set_origin(const fvec3& value) override { };
    virtual void set_extent(float value) override { assert(false); };

    virtual uint32_t get_hash() const override;
    virtual hscene* clone() const override;
    virtual void load_resources() override;
    virtual void pre_render() override;

    void add(hhittable_base* object);
    void remove(int object_id);

    void build_boxes();
    void update_materials();
    void query_lights();
    hhittable_base* get_random_light();

    // Persistent members
    std::vector<hhittable_base*> objects;

    // Runtime members
    int32_t lights_num = 0;
    std::array<hhittable_base*, MAX_LIGHTS> lights;
  };
}