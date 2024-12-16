#pragma once

#include <string>

#include "core/core.h"
#include "core/com_pointer.h"


struct ID3D10Blob;
struct IDxcBlob;

namespace engine
{
  struct ENGINE_API fshader_tools
  {
    static bool load_compiled_shader(const std::string& file_name, ComPtr<IDxcBlob>& out_shader_blob);
  
    static bool load_and_compile_hlsl(const std::string& hlsl_file_name, const std::string& entrypoint, const std::string& target, ComPtr<IDxcBlob>& out_shader_blob, std::string& out_has);

#if USE_FXC
    static bool ENGINE_API load_hlsl_fxc(const std::string& file_name, const std::string& entrypoint, const std::string& target, ComPtr<ID3D10Blob>& out_shader_blob);
#endif
  };
}
