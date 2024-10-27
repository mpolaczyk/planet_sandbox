#pragma once

#include <string>
#include <vector>

#include "core/core.h"

namespace engine
{
  class ENGINE_API fio
  {
    static std::string project_name;
    static std::string mesh_extension;
    static std::string material_extension;
    static std::string texture_extension;
    static std::string pixel_shader_extension;
    static std::string vertex_shader_extension;
    static std::string working_dir;
    static std::string workspace_dir;
    static std::string project_dir;
    static std::string materials_dir;
    static std::string meshes_dir;
    static std::string textures_dir;
    static std::string shaders_dir;

  public:
    static std::string get_project_name();
    static void init(const std::string& name);

    // Extensions
    static std::string get_mesh_extension();
    static std::string get_material_extension();
    static std::string get_texture_extension();
    static std::string get_pixel_shader_extension();
    static std::string get_vertex_shader_extension();
    
    // Directories
    static std::string get_working_dir();
    static std::string get_workspace_dir();
    static std::string get_project_dir();
    static std::string get_materials_dir();
    static std::string get_meshes_dir();
    static std::string get_textures_dir();
    static std::string get_shaders_dir();

    // Validation
    static bool validate_workspace_dir();

    // Files
    static std::string get_workspace_file_path(const char* file_name);
    static std::string get_project_file_path(const char* file_name);
    static std::string get_material_file_path(const char* file_name);
    static std::string get_mesh_file_path(const char* file_name);
    static std::string get_texture_file_path(const char* file_name);
    static std::string get_shader_file_path(const char* file_name);
    static std::string get_window_file_path();  // TODO This is not engine, move out
    static std::string get_scene_file_path();
    static std::string get_imgui_file_path();   // TODO This is not engine, move out

    // Asset discovery
    static std::vector<std::string> discover_files(const std::string& path, const std::string& extension, bool include_extension = true);
    static std::vector<std::string> discover_material_files(bool include_extension = true);
    static std::vector<std::string> discover_texture_files(bool include_extension = true);
    static std::vector<std::string> discover_mesh_files(bool include_extension = true);
    static std::vector<std::string> discover_pixel_shader_files(bool include_extension = true);
    static std::vector<std::string> discover_vertex_shader_files(bool include_extension = true);
  };
}
