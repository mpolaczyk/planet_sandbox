#pragma once

#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"
#include "renderer/gpu_resources.h"

namespace engine
{
  class ENGINE_API ashader : public aasset_base
  {
  public:
    OBJECT_DECLARE(ashader, aasset_base)
    OBJECT_DECLARE_VISITOR
    
    virtual std::string get_folder() const override;
    virtual bool load(const std::string& name) override;
    virtual void save() override;
    
    // JSON persistent
    std::string shader_file_name; // hlsl file
    std::string entrypoint;
    std::string target;
    std::string cache_file_name;  // cso file

    // Runtime
    fshader_resource resource;
    int64_t hlsl_file_timestamp = 0;
    bool compilation_successful = false;
    bool hot_swap_requested = false;
    bool hot_swap_done = false;
  };
}
