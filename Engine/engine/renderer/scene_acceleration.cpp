#include "scene_acceleration.h"

#include "engine/log.h"
#include "hittables/light.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "engine/math/math.h"
#include "core/rtti/object_registry.h"
#include "assets/texture.h"
#include "assets/mesh.h"
#include "assets/material.h"
namespace engine
{
  constexpr int32_t GPU_INDEX_NONE = -1;
  
  void fscene_acceleration::pre_frame_clear()
  {
    h_meshes.clear();

    object_buffer.clear();
    
    next_GPU_material_id = 0;
    material_map.clear();
    materials_buffer.clear();
    materials_buffer.reserve(MAX_MATERIALS);

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

  void fscene_acceleration::build_texture_buffer()
  {
    // Register all possible textures
    static bool textures_registered = false;
    if(!textures_registered)
    {
      for(atexture* a_texture : REG.get_all_by_type<atexture>())
      {
        register_texture(a_texture);
      }
      textures_registered = true;
    }
  }
  
  void fscene_acceleration::build_scene_buffers(hscene* scene)
  {
    pre_frame_clear();
    
    fsoft_asset_ptr<amaterial> default_material_asset;
    default_material_asset.set_name("default");
    amaterial* default_material = default_material_asset.get();

    // Traverse the scene
    for(hhittable_base* hittable : scene->objects)
    {
      if(hittable->get_class() == hstatic_mesh::get_class_static())
      {
        // Find base objects: hittable mesh and asset, then material and texture
        hstatic_mesh* h_mesh = static_cast<hstatic_mesh*>(hittable);
        astatic_mesh* a_mesh = h_mesh->mesh_asset_ptr.get();
        if(!a_mesh) { continue; }
        amaterial* a_material = h_mesh->material_asset_ptr.get();
        if(!a_material) { a_material = default_material; }
        atexture* a_texture = a_material->texture_asset_ptr.get();
        
        // Find material GPU id or register new one -> save in object
        int32_t material_gpu_id = get_material_gpu_id(a_material);
        if(material_gpu_id == GPU_INDEX_NONE)
        {
          material_gpu_id = register_material(a_material);
        }

        // Find texture GPU id or register new one -> save in material
        if(a_texture)
        {
          int32_t texture_gpu_id = get_texture_gpu_id(a_texture);
          materials_buffer[material_gpu_id].texture_id = texture_gpu_id;
        }
        else
        {
          materials_buffer[material_gpu_id].texture_id = GPU_INDEX_NONE;
        }

        // Build objects data buffer
        fobject_data object_data;
        h_mesh->get_object_matrices(scene->camera.view_projection, object_data);
        h_meshes.push_back(h_mesh);
        object_data.material_id = material_gpu_id;
        object_buffer.push_back(object_data);
      }
      else if(hittable->get_class() == hlight::get_class_static())
      {
        hlight* light = static_cast<hlight*>(hittable);
        if(light->properties.enabled)
        {
          lights_buffer.push_back(light->properties);
          fmath::to_xmfloat4(light->origin, lights_buffer.back().position);
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

  uint32_t fscene_acceleration::get_num_meshes() const
  {
    return fmath::to_uint32(h_meshes.size());
  }

  hstatic_mesh* fscene_acceleration::get_mesh(uint32_t index) const
  {
    return h_meshes[index];
  }

  uint32_t fscene_acceleration::get_num_textures() const
  {
    return fmath::to_uint32(a_textures.size());
  }

  atexture* fscene_acceleration::get_first_texture() const
  {
    return a_textures[0];
  }

  atexture* fscene_acceleration::get_texture(uint32_t index) const
  {
    return a_textures[index];
  }

  const fobject_data* fscene_acceleration::get_object_data(uint32_t index) const
  {
    return &object_buffer[index];
  }

  const fmaterial_properties* fscene_acceleration::get_material_properties() const
  {
    return materials_buffer.data();
  }

  const flight_properties* fscene_acceleration::get_light_properties() const
  {
    return lights_buffer.data();
  }
}