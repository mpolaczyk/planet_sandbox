#pragma once

#include <string>

#include "hittables.h"
#include "math/vec3.h"
#include "renderer/aligned_structs.h"

namespace engine
{
    class ENGINE_API hlight : public hhittable_base
    {
    public:
        OBJECT_DECLARE(hlight, hhittable_base)
        OBJECT_DECLARE_VISITOR

        virtual uint32_t get_hash() const override;
        virtual hlight* clone() const override;

        flight_properties properties;
    };
}