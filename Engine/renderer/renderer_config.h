#pragma once

#include <cstdint>

#include "engine/hash.h"

namespace engine
{
    class oclass_object;
    
    class ENGINE_API frenderer_config
    {
    public:
        // Runtime members
        const oclass_object* new_type = nullptr;
        int show_emissive = 1;
        int show_ambient = 1;
        int show_specular = 1;
        int show_diffuse = 1;
        int show_normals = 0;
        int show_object_id = 0;
        
        // Persistent members
        const oclass_object* type = nullptr;
        int resolution_vertical = 0;
        int resolution_horizontal = 0;

        inline uint32_t get_hash() const
        {
            return fhash::combine(resolution_vertical, resolution_horizontal);
        }
    };
}
