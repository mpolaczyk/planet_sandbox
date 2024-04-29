#pragma once

#include <string>
#include <vector>

#include "hittables.h"
#include "math/vec3.h"

namespace engine
{
  class hlight;
  
  class ENGINE_API hscene : public hhittable_base
  {
  public:
    OBJECT_DECLARE(hscene, hittable)
    OBJECT_DECLARE_VISITOR

    virtual uint32_t get_hash() const override;
    virtual hscene* clone() const override;
    virtual void load_resources() override;

    void add(hhittable_base* object);
    void remove(int object_id);
    
    void update_materials();
    std::vector<const hlight*> query_lights() const;

    // Persistent members
    std::vector<hhittable_base*> objects;
  };
}