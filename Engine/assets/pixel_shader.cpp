#include <fstream>
#include <sstream>

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include "renderer/dx12_lib.h"

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

  // TODO: All of this can be moved to the parent class
  
  void apixel_shader::save(apixel_shader* object)
  {
    assert(object != nullptr);

    nlohmann::json j;
    object->accept(vserialize_object(j));

    std::ostringstream oss;
    oss << object->file_name << ".pixel_shader";
    std::ofstream o(fio::get_shader_file_path(oss.str().c_str()), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if(o.is_open())
    {
      o.write(str.data(), str.length());
    }
    else
    {
      LOG_ERROR("Unable to save file {0}", oss.str());
    }
    o.close();
  }
  
  bool apixel_shader::load(apixel_shader* instance, const std::string& name)
  {
    aasset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading pixel shader: {0}", name);

    std::ostringstream oss;
    oss << name << ".pixel_shader";
    const std::string file_path = fio::get_shader_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if(input_stream.fail())
    {
      LOG_ERROR("Unable to open pixel shader asset: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    instance->accept(vdeserialize_object(j));
    instance->set_display_name(name);

    if(load_shader_cache(instance->cache_file_name, instance->render_state.blob))
    {
      return true;
    }
    std::string new_cache;
    if(!load_hlsl_dxc(instance->shader_file_name, instance->entrypoint, instance->target, instance->render_state.blob, new_cache))
    {
      DX_RELEASE(instance->render_state.blob)
      return false;
    }
    if(new_cache != "" && instance->cache_file_name != new_cache)
    {
      instance->cache_file_name = new_cache;
      apixel_shader::save(instance);
    }
    LOG_INFO("Compilation successful.");
    return true;
  }
}
