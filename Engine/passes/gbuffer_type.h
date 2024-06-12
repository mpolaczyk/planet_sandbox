#pragma once

namespace engine
{
  enum egbuffer_type
  {
    position,
    normal,
    tex_color,
    material_id,
    object_id,        // Selection mechanism.
    is_selected,      // I hate it, too expensive. TODO I need to do a proper trace.
    count,
  };
}