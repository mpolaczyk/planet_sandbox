
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
  OBJECT_DEFINE(vertex_shader_asset, asset_base, Vertex shader asset)
  OBJECT_DEFINE_SPAWN(vertex_shader_asset)
  OBJECT_DEFINE_VISITOR(vertex_shader_asset)
  
  bool vertex_shader_asset::load(vertex_shader_asset* instance, const std::string& name)
  {
    asset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading vertex shader: {0}", name);

    std::ostringstream oss;
    oss << name << ".json";
    const std::string file_path = io::get_shader_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open vertex shader asset: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    instance->accept(deserialize_object(j));
    
    if(!load_hlsl(instance->shader_file_name, instance->entrypoint, instance->target, &instance->shader_blob))
    {
      instance->shader_blob->Release();
      return false;
    }

    HRESULT result = dx11::instance().device->CreateVertexShader(instance->shader_blob->GetBufferPointer(), instance->shader_blob->GetBufferSize(), nullptr, &instance->shader);
    if (FAILED(result))
    {
      LOG_ERROR("Unable to create vertex shader: {0}", instance->shader_file_name);
      instance->shader_blob->Release();
      return false;
    }
    return true;
  }
}
