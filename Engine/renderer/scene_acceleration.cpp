#include "scene_acceleration.h"

#include "engine/log.h"
#include "hittables/light.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "math/math.h"

namespace engine
{
  constexpr int32_t GPU_INDEX_NONE = -1;
  
  void fscene_acceleration::clear()
  {
    h_meshes.clear();

    object_buffer.clear();
    
    next_GPU_material_id = 0;
    material_map.clear();
    materials_buffer.clear();
    materials_buffer.reserve(MAX_MATERIALS);
    
    next_GPU_texture_id = 0;
    texture_map.clear();
    a_textures.clear();
    a_textures.reserve(MAX_TEXTURES);

    lights_buffer.clear();
    lights_buffer.reserve(MAX_LIGHTS);
  }

  int32_t fscene_acceleration::get_material_gpu_id(amaterial* material)
  {
    return material_map.contains(material) ? material_map[material] : GPU_INDEX_NONE;
  }

  int32_t fscene_acceleration::get_texture_gpu_id(atexture* texture)
  {
    return texture_map.contains(texture) ? texture_map[texture] : GPU_INDEX_NONE;
  }

  int32_t fscene_acceleration::register_material(amaterial* material)
  {
    material_map.insert(std::pair<amaterial*, uint32_t>(material, next_GPU_material_id));
    materials_buffer.push_back(material->properties);
    return next_GPU_material_id++;
    
  }
  
  int32_t fscene_acceleration::register_texture(atexture* texture)
  {
    texture_map.insert(std::pair<atexture*, uint32_t>(texture, next_GPU_texture_id));
    a_textures.push_back(texture);
    return next_GPU_texture_id++;
  }
  
  void fscene_acceleration::build(hscene* scene, amaterial* default_material)
  {
    uint32_t scene_hash = scene->get_hash();
    bool scene_structure_changed = previous_scene_hash != scene_hash;
    previous_scene_hash = scene_hash;

    if(scene_structure_changed)
    {
      clear();
    }

    for(hhittable_base* hittable : scene->objects)
    {
      if(hittable->get_class() == hstatic_mesh::get_class_static())
      {
        // Find base objects: hittable mesh
        hstatic_mesh* h_mesh = static_cast<hstatic_mesh*>(hittable);

        // Build objects data buffer
        fobject_data object_data;
        h_mesh->get_object_matrices(scene->camera_config.view_projection, object_data);
        if(!scene_structure_changed)
        {
          continue; // TOFIX This des not work, modify the object_buffer!
        }

        // Find other base objects: mesh asset, material asset and texture asset
        astatic_mesh* a_mesh = h_mesh->mesh_asset_ptr.get();
        if(!a_mesh)
        {
          continue;
        }
        h_meshes.push_back(h_mesh);
        amaterial* a_material = h_mesh->material_asset_ptr.get();
        if(!a_material)
        {
          a_material = default_material;
        }
        atexture* a_texture = a_material->texture_asset_ptr.get();
        if(!a_texture)
        {
          a_texture = default_material->texture_asset_ptr.get();
        }
        
        // Find material GPU id or register new one -> save in object
        int32_t material_gpu_id = get_material_gpu_id(a_material);
        if(material_gpu_id == GPU_INDEX_NONE)
        {
          material_gpu_id = register_material(a_material);
        }
        object_data.material_id = material_gpu_id;
        object_buffer.push_back(object_data);
        
        // Find texture GPU id or register new one -> save in material
        int32_t texture_gpu_id = get_texture_gpu_id(a_texture);
        if(texture_gpu_id == GPU_INDEX_NONE)
        {
          texture_gpu_id = register_texture(a_texture);
        }
        materials_buffer[material_gpu_id].texture_id = texture_gpu_id;
      }
      else if(hittable->get_class() == hlight::get_class_static())
      {
        if(scene_structure_changed)
        {
          hlight* light = static_cast<hlight*>(hittable);
          if(light->properties.enabled)
          {
            lights_buffer.push_back(light->properties);
            lights_buffer.back().position = fmath::to_xmfloat4(light->origin);
          }
        }
      }
    }
  }

  bool fscene_acceleration::validate() const
  {
    if(lights_buffer.size() == 0)
    {
      LOG_ERROR("Scene is missing light");
      return false;
    }
    if(lights_buffer.size() >= MAX_LIGHTS)
    {
      LOG_ERROR("Maximum lights limit reached.");
      return false;
    }
    if(materials_buffer.size() >= MAX_MATERIALS)
    {
      LOG_ERROR("Maximum materials limit reached.");
      return false;
    }
    if(a_textures.size() >= MAX_TEXTURES)
    {
      LOG_ERROR("Maximum textures limit reached.");
      return false;
    }
    return true; 
  }
}