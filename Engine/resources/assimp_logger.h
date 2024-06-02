#pragma once

#include "assimp/DefaultLogger.hpp"

#include "core/core.h"

namespace engine
{
  struct ENGINE_API fassimp_logger : public Assimp::LogStream
  {
    static void initialize();
    void write(const char* message) override;
  };
}
