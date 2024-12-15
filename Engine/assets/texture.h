#pragma once

#include "core/core.h"

#include "core/rtti/object.h"
#include "engine/asset/asset.h"
#include "engine/renderer/gpu_resources.h"
  
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

    // Runtime data
    std::vector<float> data_hdr;
    std::vector<uint8_t> data_ldr;
    bool is_hdr = false;
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t element_size;
    DXGI_FORMAT format;

    bool is_online = false;
    ftexture_resource gpu_resource;
  };
}
