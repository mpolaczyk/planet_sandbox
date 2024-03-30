#pragma once

#include <string>
#include <vector>

#include "core/core.h"

namespace engine
{
  class ENGINE_API fio
  {
  public:
    // Directories
    static std::string get_working_dir();
    static std::string get_workspace_dir();
    static std::string get_content_dir();
    static std::string get_materials_dir();
    static std::string get_meshes_dir();
    static std::string get_textures_dir();
    static std::string get_shaders_dir();
    static std::string get_renders_dir();

    // Validation
    static bool validate_workspace_dir();

    // Files
    static std::string get_workspace_file_path(const char* file_name);
    static std::string get_renders_file_path(const char* file_name);
    static std::string get_material_file_path(const char* file_name);
    static std::string get_mesh_file_path(const char* file_name);
    static std::string get_texture_file_path(const char* file_name);
    static std::string get_shader_file_path(const char* file_name);
    static std::string get_window_file_path();
    static std::string get_scene_file_path();
    static std::string get_rendering_file_path();
    static std::string get_imgui_file_path();
    static std::string get_render_output_file_path();

    // Asset discovery
    static std::vector<std::string> discover_files(const std::string& path, const std::string& extension, bool include_extension = true);
    static std::vector<std::string> discover_material_files(bool include_extension = true);
    static std::vector<std::string> discover_texture_files(bool include_extension = true);
    static std::vector<std::string> discover_mesh_files(bool include_extension = true);
  };
}
