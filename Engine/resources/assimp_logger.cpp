#include "assimp_logger.h"

#include "engine/log.h"

namespace engine
{
   
  void fassimp_logger::initialize()
  {
    if(Assimp::DefaultLogger::isNullLogger())
    {
      Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
      Assimp::DefaultLogger::get()->attachStream(new fassimp_logger, Assimp::Logger::Err | Assimp::Logger::Warn);
    }
  }

  void fassimp_logger::write(const char* message)
  {
    LOG_ERROR("Assimp: {0}", message);
  }
}