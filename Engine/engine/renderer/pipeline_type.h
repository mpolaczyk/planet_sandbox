#pragma once

namespace engine
{
  enum epipeline_type : int
  {
    undefined = 0,
    rasterization,
    compute,      // TODO Not implemented yet
    ray_tracing,
    num
  };
}