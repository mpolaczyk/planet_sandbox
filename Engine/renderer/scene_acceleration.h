#pragma once

#include <unordered_map>

#include "renderer/aligned_structs.h"

namespace engine
{
  class astatic_mesh;
  class amaterial;
  class atexture;
  class hstatic_mesh;
  class hlight;
  class hhittable_base;

  struct fscene_acceleration
  {
    void clean();
    void build(const std::vector<hhittable_base*>& objects);
    bool validate() const;

    // Map material pointer to material id (index to materials array)
    typedef std::unordered_map<const amaterial*, uint32_t> material_map_type;
    typedef std::unordered_map<const atexture*, uint32_t> texture_map_type;
    material_map_type material_map;
    texture_map_type texture_map;

    // Order of materials added to the material_map
    fmaterial_properties materials[MAX_MATERIALS];
    uint32_t next_material_id = 0;
    uint32_t next_texture_id = 0;

    // All static meshes on the scene
    std::vector<hstatic_mesh*> meshes;
    std::vector<astatic_mesh*> assets;
    std::vector<atexture*> textures;
    
    // All lights on the scene
    flight_properties lights[MAX_LIGHTS];
    uint32_t next_light_id = 0;
  };
}