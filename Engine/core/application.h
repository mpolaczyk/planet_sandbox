#pragma once

#include "core/core.h"

namespace engine
{
  class ENGINE_API application
  {
  public:
   
    virtual void run();
  };

  application* create_appliation();
}


