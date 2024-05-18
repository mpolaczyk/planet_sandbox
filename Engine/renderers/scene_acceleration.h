#pragma once

#include <unordered_map>

namespace engine
{
  class amaterial;
  class hstatic_mesh;
  class hlight;
  class hhittable_base;
  struct fmaterial_properties;
  struct flight_properties;

  struct fscene_acceleration
  {
    void clean(int max_lights, int max_materials);
    void build(const std::vector<hhittable_base*>& objects);
    bool validate() const;
    void get_materials_array(fmaterial_properties* out_materials_array) const;
    void get_lights_array(flight_properties* out_lights_array) const;

    // Map material pointer to material id (sent to gpu)
    // Needs to be quick to search
    typedef std::unordered_map<const amaterial*, uint32_t> material_map_type;
    material_map_type material_map;

    // Order of materials added to the material_map
    std::vector<const amaterial*> materials;
    uint32_t next_material_id = 0;
    int32_t max_materials = 0;

    // All static meshes on the scene
    std::vector<const hstatic_mesh*> meshes;

    // All lights on the scene
    std::vector<const hlight*> lights;
    uint32_t next_light_id = 0;
    int32_t max_lights = 0;
  };
}