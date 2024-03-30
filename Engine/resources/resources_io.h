#pragma once

#include "core/core.h"

#include <string>

struct ID3D10Blob;

namespace engine
{
  class astatic_mesh;
  class atexture;
  
  bool ENGINE_API load_obj(const std::string& file_name, astatic_mesh* out_static_mesh);

  bool ENGINE_API load_img(const std::string& file_name, int desired_channels, atexture* out_texture);

  bool ENGINE_API load_hlsl(const std::string& file_name, const std::string& entrypoint, const std::string& target, ID3D10Blob** out_shader_blob);
}

