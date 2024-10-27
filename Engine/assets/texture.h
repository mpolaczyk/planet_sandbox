#pragma once

#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"
#include "renderer/render_state.h"

namespace engine
{
  class ENGINE_API atexture : public aasset_base
  {
  public:
    OBJECT_DECLARE(atexture, aasset_base)
    OBJECT_DECLARE_VISITOR

    virtual bool load(const std::string& name) override;
    virtual void save() override {};
    
    virtual const char* get_extension() override
    {
      return ".texture";
    }
    
    // JSON persistent
    std::string img_file_name;

    // Image file data
    int width;
    int height;
    int channels;
    ftexture_render_state render_state;
  };
}
