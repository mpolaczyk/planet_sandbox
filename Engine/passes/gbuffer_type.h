#pragma once

namespace engine
{
  enum egbuffer_type
  {
    position,
    normal,
    tex_color,
    material_id,      // Material id for the bindless table.
    object_id,        // Selection mechanism. Unique object id, color.
    is_selected,      // 0 if not, 1 if selected. I hate it, too expensive. TODO I need to do a proper trace.
    count,
  };
}