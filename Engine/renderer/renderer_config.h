#pragma once

#include <cstdint>

#include "engine/hash.h"

namespace engine
{
    class oclass_object;
    
    class ENGINE_API frenderer_config
    {
    public:
        // How work is processed
        const oclass_object* type = nullptr;
        const oclass_object* new_type = nullptr;   // For UI

        int resolution_vertical = 0;
        int resolution_horizontal = 0;

        inline uint32_t get_hash() const
        {
            return fhash::combine(resolution_vertical, resolution_horizontal);
        }
    };
}
