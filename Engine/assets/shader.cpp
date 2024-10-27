#include <fstream>
#include <sstream>

#include "renderer/dx12_lib.h"

#include "nlohmann/json.hpp"

#include "assets/pixel_shader.h"

#include "engine/log.h"
#include "engine/io.h"
#include "resources/shader_tools.h"
#include "object/object_registry.h"
#include "persistence/object_persistence.h"

#include "assets/shader.h"

namespace engine
{
  OBJECT_DEFINE(ashader, aasset_base, Shader asset)
  OBJECT_DEFINE_NOSPAWN(ashader)
  OBJECT_DEFINE_VISITOR(ashader)
  
  const char* ashader::get_folder() const
  {
    return fio::get_shaders_dir().c_str();
  }
  
  bool ashader::load(const std::string& name)
  {
    aasset_base::load(name);

    LOG_DEBUG("Loading shader: {0} {1}", name, get_extension());

    std::ostringstream oss;
    oss << name << get_extension();
    const std::string file_path = fio::get_shader_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if(input_stream.fail())
    {
      LOG_ERROR("Unable to load shader: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    accept(vdeserialize_object(j));
    set_display_name(name);

    if(fshader_tools::load_compiled_shader(cache_file_name, render_state.blob))
    {
      return true;
    }
    std::string new_cache_file_name;
    if(!fshader_tools::load_and_compile_hlsl(shader_file_name, entrypoint, target, render_state.blob, new_cache_file_name))
    {
      DX_RELEASE(render_state.blob)
      return false;
    }
    if(new_cache_file_name != "" && cache_file_name != new_cache_file_name)
    {
      cache_file_name = new_cache_file_name;
      save();
    }
    LOG_INFO("Compilation successful.");
    return true;
  }

  void ashader::save()
  {
    nlohmann::json j;
    accept(vserialize_object(j));

    std::ostringstream oss;
    oss << file_name << get_extension();
    std::ofstream o(fio::get_shader_file_path(oss.str().c_str()), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if(o.is_open())
    {
      o.write(str.data(), str.length());
    }
    else
    {
      LOG_ERROR("Unable to save shader: {0}", oss.str());
    }
    o.close();
  }
}