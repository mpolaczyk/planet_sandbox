

#include "object/factories.h"

#include "renderers/cpu_renderer_preview.h"
#include "renderers/cpu_renderer_normals.h"
#include "renderers/cpu_renderer_faces.h"
#include "renderers/cpu_renderer_reference.h"
#include "engine/log.h"

namespace engine
{
  cpu_renderer_base* object_factory::spawn_renderer(renderer_type type)
  {
    if (type > renderer_type::num)
    {
      LOG_ERROR("Unable to spawn a renderer of type: {0}", static_cast<int32_t>(type));
      return nullptr;
    }
    if (type == renderer_type::preview) { return cpu_renderer_preview::spawn(""); }
    else if (type == renderer_type::preview_normals) { return cpu_renderer_normals::spawn(""); }
    else if (type == renderer_type::preview_faces) { return cpu_renderer_faces::spawn(""); }
    else if (type == renderer_type::reference) { return cpu_renderer_reference::spawn(""); }
    return nullptr;
  }

  hittable* object_factory::spawn_hittable(hittable_type type)
  {
    if (type > hittable_type::num)
    {
      LOG_ERROR("Invalid hittable type: {0}", static_cast<int32_t>(type));
      return nullptr;
    }

    hittable* obj = nullptr;
    if (type == hittable_type::scene) { obj = new scene(); }
    else if (type == hittable_type::static_mesh) { obj = new static_mesh(); }
    else if (type == hittable_type::sphere) { obj = new sphere(); }
    else
    {
      LOG_ERROR("Unable to spawn a hittable of type: {0}", static_cast<int32_t>(type));
      return nullptr;
    }
    
    // Unique id for hittables
    if (obj != nullptr)
    {
      obj->id = hittable::last_id;
      hittable::last_id++;
    }
    return obj;
  }
}