#pragma once

#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"
#include "renderer/render_state.h"

namespace engine
{
  class ENGINE_API ashader : public aasset_base
  {
  public:
    OBJECT_DECLARE(ashader, aasset_base)
    OBJECT_DECLARE_LOAD(ashader)
    OBJECT_DECLARE_SAVE(ashader)
    OBJECT_DECLARE_VISITOR
    
    // JSON persistent
    std::string shader_file_name; // hlsl file
    std::string entrypoint;
    std::string target;
    std::string cache_file_name;  // cso file

    // Runtime
    fshader_render_state render_state;
  };
}
