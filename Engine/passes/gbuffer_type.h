#pragma once

namespace engine
{
  enum egbuffer_type
  {
    position,
    normal,
    material_id,      // Material id for the bindless table.
    uv,
    count,
  };
}