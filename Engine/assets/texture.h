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

    virtual std::string get_extension() const override;
    virtual std::string get_folder() const override;
    virtual bool load(const std::string& name) override;
    virtual void save() override {};
    
    // JSON persistent
    std::string img_file_name;
    
    // Offline data
    std::vector<float> data_hdr;
    std::vector<uint8_t> data_ldr;
    bool is_hdr = false;
    
    ftexture_resource gpu_resource;
  };
}
