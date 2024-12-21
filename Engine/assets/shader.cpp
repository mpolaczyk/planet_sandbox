#include "stdafx.h"

#include "engine/renderer/dx12_lib.h"

#include "assets/pixel_shader.h"

#include "engine/io.h"
#include "engine/resources/shader_tools.h"
#include "engine/persistence/object_persistence.h"

#include "assets/shader.h"

namespace engine
{
  OBJECT_DEFINE(ashader, aasset_base, Shader asset)
  OBJECT_DEFINE_NOSPAWN(ashader)
  OBJECT_DEFINE_VISITOR(ashader)
  
  std::string ashader::get_folder() const
  {
    return fio::get_shaders_dir().c_str();
  }
  
  bool ashader::load(const std::string& name)
  {
    LOG_DEBUG("Loading shader: {0} {1}", name, get_extension())

    aasset_base::load(name);
    
    std::ostringstream oss;
    oss << name << get_extension();
    const std::string file_path = fio::get_shader_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if(input_stream.fail())
    {
      LOG_ERROR("Unable to load shader: {0}", file_path)
      compilation_successful = false;
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    accept(vdeserialize_object(j));
    set_display_name(name);
    hlsl_file_timestamp = fio::get_last_write_time(fio::get_shader_file_path(shader_file_name.c_str()).c_str());

    DX_RELEASE(resource.blob)
    if(fshader_tools::load_compiled_shader(cache_file_name, resource.blob))
    {
      compilation_successful = true;
      return true;
    }
    std::string new_cache_file_name;
    if(!fshader_tools::load_and_compile_hlsl(shader_file_name, entrypoint, target, resource.blob, new_cache_file_name))
    {
      compilation_successful = false;
      return false;
    }
    if(new_cache_file_name.empty() && cache_file_name != new_cache_file_name)
    {
      cache_file_name = new_cache_file_name;
      save();
    }
    LOG_INFO("Compilation successful.")
    compilation_successful = true;
    return true;
  }

  void ashader::save()
  {
    nlohmann::json j;
    accept(vserialize_object(j));

    std::ostringstream oss;
    oss << name << get_extension();
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