#pragma once

#include "core/core.h"

namespace ofbx
{
    struct Mesh;
}
namespace engine
{
    struct ENGINE_API ffbx
    {
        static bool save_as_obj(const ofbx::Mesh& mesh, const char* path);
        
        static void load_fbx(class hscene* scene_object);
    };
}
