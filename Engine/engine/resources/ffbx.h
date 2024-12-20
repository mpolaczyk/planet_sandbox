#pragma once

#include <string>

#include "core/core.h"

struct aiMesh;
struct aiMaterial;

namespace engine
{
  struct fmaterial_properties;
  class hscene;
  
  struct ENGINE_API ffbx
  {
    //static void save_as_obj_ofbx(const ofbx::Mesh& mesh, const char* path);
    static void save_as_obj_assimp(aiMesh* mesh, aiMaterial* material, const char* path);

    //static void load_fbx_ofbx(const std::string& file_name, hscene* scene_object);
    static void load_fbx_assimp(const std::string& file_name, hscene* scene_object);

  private:
    //static void import_material_from_blender_4_1_principled_brdf(const ofbx::Material* material, fmaterial_properties& out_material_properties);
    //static void import_material_from_unity_2022_3_urp(const ofbx::Material* material, fmaterial_properties& out_material_properties);
  };
}
