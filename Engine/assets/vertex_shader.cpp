
#include <fstream>
#include <sstream>

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include "renderer/dx11_lib.h"

#include "nlohmann/json.hpp"

#include "assets/vertex_shader.h"

#include "engine/log.h"
#include "engine/io.h"
#include "resources/resources_io.h"
#include "object/object_registry.h"
#include "persistence/object_persistence.h"

namespace engine
{
  OBJECT_DEFINE(avertex_shader, aasset_base, Vertex shader asset)
  OBJECT_DEFINE_SPAWN(avertex_shader)
  OBJECT_DEFINE_VISITOR(avertex_shader)
  
  bool avertex_shader::load(avertex_shader* instance, const std::string& name)
  {
    aasset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading vertex shader: {0}", name);

    std::ostringstream oss;
    oss << name << ".json";
    const std::string file_path = fio::get_shader_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open vertex shader asset: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    instance->accept(deserialize_object(j));
    instance->set_display_name(name);
    
    if(!load_hlsl(instance->shader_file_name, instance->entrypoint, instance->target, instance->shader_blob))
    {
      DX_RELEASE(instance->shader_blob)
      return false;
    }

    if(FAILED(fdx11::instance().device->CreateVertexShader(instance->shader_blob->GetBufferPointer(), instance->shader_blob->GetBufferSize(), nullptr, instance->shader.GetAddressOf())))
    {
      LOG_ERROR("Unable to create vertex shader: {0}", instance->shader_file_name);
      return false;
    }
    return true;
  }
}
