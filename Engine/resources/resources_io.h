#pragma once

#include "core/core.h"

#include <string>

struct ID3D10Blob;

namespace engine
{
  class static_mesh_asset;
  class texture_asset;
  
  bool ENGINE_API load_obj(const std::string& file_name, int shape_index, static_mesh_asset* out_static_mesh);

  bool ENGINE_API load_img(const std::string& file_name, int desired_channels, texture_asset* out_texture);

  bool ENGINE_API load_hlsl(const std::string& file_name, const std::string& entrypoint, const std::string& target, ID3D10Blob** out_shader_blob);
}

