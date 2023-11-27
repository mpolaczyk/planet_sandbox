

#include "object/factories.h"
#include "object/object_types.h"
#include "object/object_registry.h"
#include "engine/log.h"

#include "renderers/cpu_renderer_preview.h"
#include "renderers/cpu_renderer_normals.h"
#include "renderers/cpu_renderer_faces.h"
#include "renderers/cpu_renderer_reference.h"

#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "hittables/sphere.h"

namespace engine
{
  cpu_renderer_base* object_factory::spawn_renderer(object_type type)
  {
    /*if (!cpu_renderer_base::is_parent_of_static(type))  FIX
    {
      LOG_ERROR("Unable to spawn a renderer of type: {0}", object_type_names[static_cast<int32_t>(type)]);
      return nullptr;
    }*/
    if (type ==      object_type::cpu_renderer_preview)   { return cpu_renderer_preview::spawn(""); } // FIX UGLY this with a dynamic type system
    else if (type == object_type::cpu_renderer_normals)   { return cpu_renderer_normals::spawn(""); }
    else if (type == object_type::cpu_renderer_faces)     { return cpu_renderer_faces::spawn(""); }
    else if (type == object_type::cpu_renderer_reference) { return cpu_renderer_reference::spawn(""); }
    else
    {
      LOG_ERROR("Unable to spawn a renderer of type: {0}", static_cast<int32_t>(type));
      return nullptr;
    }
  }

  hittable* object_factory::spawn_hittable(object_type type)
  {
    /*if (!hittable::is_parent_of_static(type))  FIX
    {
      LOG_ERROR("Unable to spawn a hittable of type: {0}", object_type_names[static_cast<int32_t>(type)]);
      return nullptr;
    }*/
    hittable* obj = nullptr;
    if (type == object_type::scene) // FIX UGLY this with a dynamic type system
    { 
      obj = scene::spawn(""); 
      obj->type = object_type::scene; 
    }
    else if (type == object_type::static_mesh) 
    { 
      obj = static_mesh::spawn("");
      obj->type = object_type::static_mesh;
    }
    else if (type == object_type::sphere)      
    { 
      obj = sphere::spawn(""); 
      obj->type = object_type::sphere;
    }
    else
    {
      LOG_ERROR("Unable to spawn a hittable of type: {0}", static_cast<int32_t>(type));
      return nullptr;
    }
    assert(type == obj->get_type());
    assert(type == get_object_registry()->get_type(obj->get_runtime_id()));
    return obj;
  }
}