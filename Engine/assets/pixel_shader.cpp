
#include <fstream>
#include <sstream>

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include "renderer/dx11_lib.h"

#include "nlohmann/json.hpp"

#include "assets/pixel_shader.h"

#include "engine/log.h"
#include "engine/io.h"
#include "resources/resources_io.h"
#include "object/object_registry.h"
#include "persistence/object_persistence.h"

namespace engine
{
  OBJECT_DEFINE(apixel_shader, aasset_base, Pixel shader asset)
  OBJECT_DEFINE_SPAWN(apixel_shader)
  OBJECT_DEFINE_VISITOR(apixel_shader)
  
  bool apixel_shader::load(apixel_shader* instance, const std::string& name)
  {
    aasset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading pixel shader: {0}", name);

    std::ostringstream oss;
    oss << name << ".json";
    const std::string file_path = fio::get_shader_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open pixel shader asset: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    instance->accept(vdeserialize_object(j));
    instance->set_display_name(name);
    
    if(!load_hlsl(instance->shader_file_name, instance->entrypoint, instance->target, instance->render_state.shader_blob))
    {
      DX_RELEASE(instance->render_state.shader_blob)
      return false;
    }
    
    if(FAILED(fdx11::instance().device->CreatePixelShader(instance->render_state.shader_blob->GetBufferPointer(), instance->render_state.shader_blob->GetBufferSize(), nullptr, instance->render_state.shader.GetAddressOf())))
    {
      LOG_ERROR("Unable to create pixel shader: {0}", instance->shader_file_name);
      DX_RELEASE(instance->render_state.shader_blob)
      return false;
    }
    return true;
  }
}
