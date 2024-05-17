﻿#pragma once

#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"
#include "renderer/render_state.h"

namespace engine
{
  class ENGINE_API avertex_shader : public aasset_base
  {
  public:
    OBJECT_DECLARE(avertex_shader, aasset_base)
    OBJECT_DECLARE_LOAD(avertex_shader)
    OBJECT_DECLARE_SAVE(avertex_shader)
    OBJECT_DECLARE_VISITOR

    // JSON persistent
    std::string shader_file_name;
    std::string entrypoint;
    std::string target;

    // Runtime
    fvertex_shader_render_state render_state;
  };
}
