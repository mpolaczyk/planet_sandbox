#pragma once

#include "core/core.h"

namespace ofbx
{
    struct Mesh;
    struct Material;
}
namespace engine
{
    struct fmaterial_properties;
    
    struct ENGINE_API ffbx
    {
        static bool save_as_obj(const ofbx::Mesh& mesh, const char* path);
        
        static void load_fbx(class hscene* scene_object);

    private:
        static void import_material_from_blender_4_1_principled_brdf(const ofbx::Material* material, fmaterial_properties& out_material_properties);
    };
}
