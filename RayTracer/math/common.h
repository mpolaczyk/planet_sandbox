#pragma once

#include <vector>

#include "asset/textures.h"

#include "math/triangle_face.h"



namespace obj_helper
{
  bool load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces);
}

namespace img_helper
{
  bool load_img(const std::string& file_name, int width, int height, engine::texture* out_texture);
}

