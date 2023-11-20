

#include "object/factories.h"

#include "renderers/example_renderer.h"
#include "renderers/preview_renderer.h"
#include "renderers/preview_normals_renderer.h"
#include "renderers/preview_faces_renderer.h"
#include "renderers/reference_renderer.h"
#include "engine/log.h"

namespace engine
{
  async_renderer_base* object_factory::spawn_renderer(renderer_type type)
  {
    if (type > renderer_type::num)
    {
      LOG_ERROR("Unable to spawn a renderer of type: {0}", static_cast<int32_t>(type));
      return nullptr;
    }
    if (type == renderer_type::example) { return new example_renderer(); }
    else if (type == renderer_type::preview) { return new preview_renderer(); }
    else if (type == renderer_type::preview_normals) { return new preview_normals_renderer(); }
    else if (type == renderer_type::preview_faces) { return new preview_faces_renderer(); }
    else if (type == renderer_type::reference) { return new reference_renderer(); }
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