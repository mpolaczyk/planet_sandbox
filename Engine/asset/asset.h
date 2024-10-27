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
    virtual std::string get_extension() const = 0;
    virtual std::string get_folder() const = 0;
    virtual bool load(const std::string& name);
    virtual void save() = 0;
    
    // Persistent - part of the file name, not in JSON
    std::string name;
  };
}
