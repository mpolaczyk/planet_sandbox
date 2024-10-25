#include <fstream>
#include <sstream>


#include <d3dcompiler.h>
#include "renderer/dx12_lib.h"

#include "nlohmann/json.hpp"

#include "assets/vertex_shader.h"

#include "engine/log.h"
#include "engine/io.h"
#include "resources/shader_tools.h"
#include "object/object_registry.h"
#include "persistence/object_persistence.h"

namespace engine
{
  OBJECT_DEFINE(avertex_shader, aasset_base, Vertex shader asset)
  OBJECT_DEFINE_SPAWN(avertex_shader)
  OBJECT_DEFINE_VISITOR(avertex_shader)

  // TODO: All of this can be moved to the parent class
  
  void avertex_shader::save(avertex_shader* object)
  {
    assert(object != nullptr);

    nlohmann::json j;
    object->accept(vserialize_object(j));

    std::ostringstream oss;
    oss << object->file_name << ".vertex_shader";
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
  
  bool avertex_shader::load(avertex_shader* instance, const std::string& name)
  {
    aasset_base::load(instance, name);

    assert(instance);
    LOG_DEBUG("Loading vertex shader: {0}", name);

    std::ostringstream oss;
    oss << name << ".vertex_shader";
    const std::string file_path = fio::get_shader_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if(input_stream.fail())
    {
      LOG_ERROR("Unable to open vertex shader asset: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    instance->accept(vdeserialize_object(j));
    instance->set_display_name(name);

    if(fshader_tools::load_compiled_shader(instance->cache_file_name, instance->render_state.blob))
    {
      LOG_INFO("Loaded from cache.");
      return true;
    }
    std::string new_cache;
    if(!fshader_tools::load_and_compile_hlsl(instance->shader_file_name, instance->entrypoint, instance->target, instance->render_state.blob, new_cache))
    {
      DX_RELEASE(instance->render_state.blob)
      return false;
    }
    if(new_cache != "" && instance->cache_file_name != new_cache)
    {
      instance->cache_file_name = new_cache;
      avertex_shader::save(instance);
    }
    LOG_INFO("Compilation successful.");
    return true;
  }
}
