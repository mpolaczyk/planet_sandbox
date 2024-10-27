#pragma once

#include "core/core.h"
#include "object/object.h"

namespace engine
{
  // Base class for all assets, don't instantiate
  class ENGINE_API aasset_base : public oobject
  {
  public:
    OBJECT_DECLARE(aasset_base, oobject)

    // Interface mandatory for each asset
    virtual bool load(const std::string& name) = 0;
    virtual void save() = 0;
    virtual const char* get_extension() = 0;
    //virtual const char* get_folder() = 0; // TODO will be used for saving
    
    // JSON file name
    std::string file_name;
  };
}
