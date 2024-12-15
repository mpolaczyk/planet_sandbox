#pragma once

#include <unordered_map>

#include "engine/renderer/aligned_structs.h"

namespace engine
{
  class astatic_mesh;
  class amaterial;
  class atexture;
  class hscene;
  class hstatic_mesh;
  class hlight;
  class hhittable_base;
  
   /* Building the scene data buffers that will be sent to the GPU.
   *  Nothing sophisticated, lots of optimisation opportunities, but I don't care for now.
   *  Data is orientes in buffers that can be pushed to the GPU directly.
   *  Material ids and texture ids are trnaslated to GPU equivalents (scene is a subset of all the content available in editor).
   *  1. Objects buffer is built every frame.
   *     Objects can be added/removed dynamically.
   *  1. Lights buffer is built every frame.
   *     Lights can be added/removed dynamically.
   *  2. Materials buffer is built every frame.
   *     Materials can be added/removed as they are used or not on the scene.
   *     One material can be reused on multiple static meshes - the list is unique.
   *     Material properties (colors, textures used) can change.
   *  3. Textures are resident in GPU.
   *     All textures are loaded, independently of what is used on the scene.
   */
  struct ENGINE_API fscene_acceleration
  {
    void build_texture_buffer();
    void build_scene_buffers(hscene* scene);
    bool validate() const;
    
    inline uint32_t get_num_meshes() const;
    inline hstatic_mesh* get_mesh(uint32_t index) const;
    inline uint32_t get_num_textures() const;
    inline atexture* get_first_texture() const;
    inline atexture* get_texture(uint32_t index) const;
    inline const fobject_data* get_object_data(uint32_t index) const;
    const fmaterial_properties* get_material_properties() const;
    const flight_properties* get_light_properties() const;

  private:
    void pre_frame_clear();
    int32_t get_material_gpu_id(amaterial* material);
    int32_t get_texture_gpu_id(atexture* texture);
    int32_t register_material(amaterial* material);
    int32_t register_texture(atexture* texture);

    // All static meshes on the scene
    std::vector<hstatic_mesh*> h_meshes;
    
    // All textures in the GPU memory
    std::vector<atexture*> a_textures;
    
    // All objects on the scene
    std::vector<fobject_data> object_buffer;

    // All materials on the scene
    std::vector<fmaterial_properties> materials_buffer;

    // All lights on the scene
    std::vector<flight_properties> lights_buffer;

    // Object pointer to GPU index
    std::unordered_map<amaterial*, int32_t> material_map;
    std::unordered_map<atexture*, int32_t> texture_map;
    
    int32_t next_GPU_material_id = 0;
    int32_t next_GPU_texture_id = 0;
  };
}