#pragma once

#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"
#include "renderer/render_state.h"

namespace engine
{
  class ENGINE_API apixel_shader : public aasset_base
  {
  public:
    OBJECT_DECLARE(apixel_shader, aasset_base)
    OBJECT_DECLARE_LOAD(apixel_shader)
    OBJECT_DECLARE_SAVE(apixel_shader)
    OBJECT_DECLARE_VISITOR

    // JSON persistent
    // TODO: All of this can be moved to the parent class
    std::string shader_file_name;
    std::string entrypoint;
    std::string target;
    std::string cache_file_name;

    // Runtime
    fpixel_shader_render_state render_state;
  };
}
