#pragma once

#include "texture.h"
#include "core/core.h"

#include "object/object.h"
#include "asset/asset.h"
#include "asset/soft_asset_ptr.h"
#include "renderer/aligned_structs.h"

namespace engine
{
  using namespace DirectX;

  class ENGINE_API amaterial : public aasset_base
  {
  public:
    OBJECT_DECLARE(amaterial, aasset_base)
    OBJECT_DECLARE_VISITOR

    virtual const char* get_extension() const override;
    virtual const char* get_folder() const override;
    virtual bool load(const std::string& name) override;
    virtual void save() override;

    // JSON persistent
    fmaterial_properties properties;
    fsoft_asset_ptr<atexture> texture_asset_ptr;
  };
}
