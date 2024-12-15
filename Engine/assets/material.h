#pragma once

#include "core/core.h"

#include "core/rtti/object.h"
#include "engine/asset/asset.h"
#include "engine/asset/soft_asset_ptr.h"
#include "engine/renderer/aligned_structs.h"

namespace engine
{
  class atexture;
  
  using namespace DirectX;

  class ENGINE_API amaterial : public aasset_base
  {
  public:
    OBJECT_DECLARE(amaterial, aasset_base)
    OBJECT_DECLARE_VISITOR

    virtual std::string get_extension() const override;
    virtual std::string get_folder() const override;
    virtual bool load(const std::string& name) override;
    virtual void save() override;

    // JSON persistent
    fmaterial_properties properties;
    fsoft_asset_ptr<atexture> texture_asset_ptr;
  };
}
