#pragma once

#include <unordered_map>

#include "renderer/aligned_structs.h"

namespace engine
{
  class astatic_mesh;
  class amaterial;
  class atexture;
  class hscene;
  class hstatic_mesh;
  class hlight;
  class hhittable_base;

  /* Scene acceleration:
   * - Helps push the scene to GPU friendly buffers
   * - Translates material pointer to material GPU index
   * - Translates texture pointer to texture GPU index
   * - In case the scene structure does not change:
   *    - Rebuilds only camera view projection depnedent data
   *    - Skips the buffers rebuild
   * - Else:
   *    - Clears and rebuilds all the buffers
   * - Not clever enough to analyze the diff between this and last frame, effectively update instead of build from scratch
   */
  struct ENGINE_API fscene_acceleration
  {
    void build(hscene* scene);
    bool validate() const;
        
    // All static meshes on the scene
    std::vector<hstatic_mesh*> h_meshes;
    
    // All textures on the scene
    std::vector<atexture*> a_textures;
    
    // All objects on the scene
    std::vector<fobject_data> object_buffer;

    // All materials on the scene
    std::vector<fmaterial_properties> materials_buffer;

    // All lights on the scene
    std::vector<flight_properties> lights_buffer;

  private:
    void clear();
    
    int32_t get_material_gpu_id(amaterial* material);
    int32_t get_texture_gpu_id(atexture* texture);
    int32_t register_material(amaterial* material);
    int32_t register_texture(atexture* texture);

    // Object pointer to GPU index
    std::unordered_map<amaterial*, int32_t> material_map;
    std::unordered_map<atexture*, int32_t> texture_map;
    
    int32_t next_GPU_material_id = 0;
    int32_t next_GPU_texture_id = 0;
    uint32_t previous_scene_hash = 0;
  };
}