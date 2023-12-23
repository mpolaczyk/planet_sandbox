
#include <fstream>
#include <sstream>

#include <d3d11_1.h>
#include <d3dcompiler.h>

#include "nlohmann/json.hpp"

#include "assets/vertex_shader.h"

#include "engine/log.h"
#include "engine/io.h"
#include "resources/resources_io.h"
#include "object/object_registry.h"
#include "persistence/object_persistence.h"
#include "renderer/dx11_lib.h"

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

    ID3DBlob* vs_blob;
    ID3D11VertexShader* vs;
    {
      ID3DBlob* shader_compiler_errors_blob;
      std::wstring stemp = std::wstring(instance->shader_file_name.begin(), instance->shader_file_name.end());
      LPCWSTR sw = stemp.c_str();
      HRESULT result = D3DCompileFromFile(sw, nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vs_blob, &shader_compiler_errors_blob);
      if (FAILED(result))
      {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
          LOG_ERROR("Could not compile shader, file {0} not found.", instance->shader_file_name);
        }
        else if (shader_compiler_errors_blob)
        {
          LOG_ERROR("{0}", (const char*)shader_compiler_errors_blob->GetBufferPointer());
          shader_compiler_errors_blob->Release();
        }
        vs_blob->Release();// FIX RAII for DX11 objects
        return false;
      }

      result = dx11::instance().device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &vs);
      if (FAILED(result))
      {
        LOG_ERROR("Unable to create vertex shader: {0}", instance->shader_file_name);
        vs_blob->Release();
        return false;
      }
     vs_blob->Release();
    }
    return true;
  }
}
