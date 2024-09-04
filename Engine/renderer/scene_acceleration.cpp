#include "scene_acceleration.h"

#include "engine/log.h"
#include "hittables/light.h"
#include "hittables/static_mesh.h"
#include "math/math.h"

namespace engine
{
  void fscene_acceleration::clean()
  {
    next_material_id = 0;
    material_map.clear();
    memset(materials, 0, sizeof(fmaterial_properties)*MAX_MATERIALS);

    next_light_id = 0;
    memset(lights, 0, sizeof(flight_properties)*MAX_LIGHTS);

    meshes.clear();
    assets.clear();
  }

  void fscene_acceleration::build(const std::vector<hhittable_base*>& objects)
  {
    for(hhittable_base* hittable : objects)
    {
      if(hittable->get_class() == hstatic_mesh::get_class_static())
      {
        hstatic_mesh* mesh = static_cast<hstatic_mesh*>(hittable);
        astatic_mesh* mesh_asset = mesh->mesh_asset_ptr.get();
        if(!mesh_asset)
        {
          continue;
        }
        meshes.push_back(mesh);
        assets.push_back(mesh_asset);
        const amaterial* material = mesh->material_asset_ptr.get();
        if(material && !material_map.contains(material))
        {
          if(next_material_id < MAX_MATERIALS)
          {
            material_map.insert(std::pair<const amaterial*, uint32_t>(material, next_material_id));
            materials[next_material_id] = material->properties;
            next_material_id++;
          }
          else
          {
            LOG_ERROR("Maximum materials limit reached.");
          }
        }
      }
      else if(hittable->get_class() == hlight::get_class_static())
      {
        const hlight* light = static_cast<const hlight*>(hittable);
        if(light->properties.enabled)
        {
          if(next_light_id < MAX_LIGHTS)
          {
            lights[next_light_id] = light->properties;
            lights[next_light_id].position = fmath::to_xmfloat4(light->origin);
            next_light_id++;
          }
          else
          {
            LOG_ERROR("Maximum lights limit reached.");
          }
        }
      }
    }
  }

  bool fscene_acceleration::validate() const
  {
    if(next_light_id == 0)
    {
      LOG_ERROR("Scene is missing light");
      return false;
    }
    return true; 
  }
}