#include "scene_acceleration.h"

#include "engine/log.h"
#include "hittables/light.h"
#include "hittables/static_mesh.h"

namespace engine
{
  void fscene_acceleration::clean(int in_max_lights, int in_max_materials)
  {
    max_materials = in_max_materials;
    next_material_id = 0;
    material_map.clear();
    material_map.reserve(max_materials);
    materials.clear();
    materials.reserve(max_materials);

    max_lights = in_max_lights;
    next_light_id = 0;
    lights.clear();
    lights.reserve(max_lights);

    meshes.clear();
  }

  void fscene_acceleration::build(const std::vector<hhittable_base*>& objects)
  {
    for(const hhittable_base* hittable : objects)
    {
      if(hittable->get_class() == hstatic_mesh::get_class_static())
      {
        const hstatic_mesh* mesh = static_cast<const hstatic_mesh*>(hittable);
        meshes.push_back(mesh);
        volatile const astatic_mesh* mesh_asset = mesh->mesh_asset_ptr.get();
        const amaterial* material = mesh->material_asset_ptr.get();
        if(material && !material_map.contains(material))
        {
          material_map.insert(std::pair<const amaterial*, uint32_t>(material, next_material_id));
          materials.push_back(material);
          next_material_id++;
        }
      }
      else if(hittable->get_class() == hlight::get_class_static())
      {
        const hlight* light = static_cast<const hlight*>(hittable);
        if(light->properties.enabled)
        {
          lights.push_back(light);
          next_light_id++;
        }
      }
    }
  }

  bool fscene_acceleration::validate() const
  {
    if(next_material_id > max_materials)
    {
      LOG_ERROR("Maximum materials limit reached.");
      return false;
    }
    if(next_light_id > max_lights)
    {
      LOG_ERROR("Maximum lights limit reached.");
      return false;
    }
    if(lights.size() == 0)
    {
      LOG_ERROR("Scene is missing light");
      return false;
    }
    return true; 
  }

  void fscene_acceleration::get_materials_array(fmaterial_properties* out_materials_array) const
  {
    for(uint32_t i = 0; i < max_materials; i++)
    {
      out_materials_array[i] = i < next_material_id ? materials[i]->properties : fmaterial_properties();
    }
  }

  void fscene_acceleration::get_lights_array(flight_properties* out_lights_array) const
  {
    for(uint32_t i = 0; i < max_lights; i++)
    {
      if(i < next_light_id)
      {
        out_lights_array[i] = lights[i]->properties;
        out_lights_array[i].position = XMFLOAT4(lights[i]->origin.e);
      }
      else
      {
        out_lights_array[i] = flight_properties();
      }
    }
  }
}