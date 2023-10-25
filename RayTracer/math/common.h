#pragma once






namespace obj_helper
{
  bool load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces);
}
class texture;
namespace img_helper
{
  bool load_img(const std::string& file_name, int width, int height, texture* out_texture);
}

