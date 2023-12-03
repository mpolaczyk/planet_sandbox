

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
  cpu_renderer_base* object_factory::spawn_renderer(const class_object* type)
  {
    /*if (!cpu_renderer_base::is_parent_of_static(type))  FIX
    {
      LOG_ERROR("Unable to spawn a renderer of type: {0}", object_type_names[static_cast<int32_t>(type)]);
      return nullptr;
    }*/
    return type->spawn_instance<cpu_renderer_base>("");

    //if (type ==      cpu_renderer_preview::get_class_static())   { return cpu_renderer_preview::spawn(""); } // FIX UGLY this with a dynamic type system
    //else if (type == cpu_renderer_normals::get_class_static())   { return cpu_renderer_normals::spawn(""); }
    //else if (type == cpu_renderer_faces::get_class_static())     { return cpu_renderer_faces::spawn(""); }
    //else if (type == cpu_renderer_reference::get_class_static()) { return cpu_renderer_reference::spawn(""); }
    //else
    //{
    //  LOG_ERROR("Unable to spawn a renderer of type: {0}", type->get_name());
    //  return nullptr;
    //}
  }

  hittable* object_factory::spawn_hittable(const class_object* type)
  {
    /*if (!hittable::is_parent_of_static(type))  FIX
    {
      LOG_ERROR("Unable to spawn a hittable of type: {0}", object_type_names[static_cast<int32_t>(type)]);
      return nullptr;
    }*/

    return type->spawn_instance<hittable>("");

    //hittable* obj = nullptr;
    //if (type == scene::get_class_static()) // FIX UGLY this with a dynamic type system
    //{ 
    //  obj = scene::spawn(""); 
    //  obj->type = scene::get_class_static();
    //}
    //else if (type == static_mesh::get_class_static()) 
    //{ 
    //  obj = static_mesh::spawn("");
    //  obj->type = static_mesh::get_class_static();
    //}
    //else if (type == sphere::get_class_static())
    //{ 
    //  obj = sphere::spawn(""); 
    //  obj->type = sphere::get_class_static();
    //}
    //else
    //{
    //  LOG_ERROR("Unable to spawn a hittable of type: {0}", type->get_name());
    //  return nullptr;
    //}
    //assert(type == obj->get_class());
    //assert(type == get_object_registry()->get_class(obj->get_runtime_id()));
    //return obj;
  }
}