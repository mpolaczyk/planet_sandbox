#include <filesystem>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

#include "engine/io.h"

namespace engine
{
  std::string io::get_working_dir()
  {
    std::string current_dir = std::filesystem::current_path().string();
    std::ostringstream oss;
    oss << current_dir << "\\";
    return oss.str();
  }

  std::string io::get_workspace_dir()
  {
    std::string working_dir = get_working_dir();
    std::ostringstream oss;
    oss << working_dir << "..\\..\\Workspace\\";
    return oss.str();
  }

  std::string io::get_content_dir()
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << "Content\\";
    return oss.str();
  }

  std::string io::get_materials_dir()
  {
    std::string content_dir = get_content_dir();
    std::ostringstream oss;
    oss << content_dir << "Materials\\";
    return oss.str();
  }

  std::string io::get_meshes_dir()
  {
    std::string content_dir = get_content_dir();
    std::ostringstream oss;
    oss << content_dir << "Meshes\\";
    return oss.str();
  }

  std::string io::get_textures_dir()
  {
    std::string content_dir = get_content_dir();
    std::ostringstream oss;
    oss << content_dir << "Textures\\";
    return oss.str();
  }

  std::string io::get_shaders_dir()
  {
    std::string content_dir = get_content_dir();
    std::ostringstream oss;
    oss << content_dir << "Shaders\\";
    return oss.str();
  }
  
  std::string io::get_renders_dir()
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << "Renders\\";
    return oss.str();
  }

  bool io::validate_workspace_dir()
  {
      using namespace std;
      struct stat sb;
      if (stat(get_workspace_dir().c_str(), &sb) == 0)
      {
          return true;
      }
      return false;
  }

  std::string io::get_workspace_file_path(const char* file_name)
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << file_name;
    return oss.str();
  }

  std::string io::get_renders_file_path(const char* file_name)
  {
    std::string renders_dir = get_renders_dir();
    std::ostringstream oss;
    oss << renders_dir << file_name;
    return oss.str();
  }

  std::string io::get_material_file_path(const char* file_name)
  {
    std::string materials_dir = get_materials_dir();
    std::ostringstream oss;
    oss << materials_dir << file_name;
    return oss.str();
  }

  std::string io::get_mesh_file_path(const char* file_name)
  {
    std::string meshes_dir = get_meshes_dir();
    std::ostringstream oss;
    oss << meshes_dir << file_name;
    return oss.str();
  }

  std::string io::get_texture_file_path(const char* file_name)
  {
    std::string textures_dir = get_textures_dir();
    std::ostringstream oss;
    oss << textures_dir << file_name;
    return oss.str();
  }

  std::string io::get_shader_file_path(const char* file_name)
  {
    std::string shaders_dir = get_shaders_dir();
    std::ostringstream oss;
    oss << shaders_dir << file_name;
    return oss.str();
  }
  
  std::string io::get_window_file_path()
  {
    return get_workspace_file_path("window.json");
  }

  std::string io::get_scene_file_path()
  {
    return get_workspace_file_path("scene.json");
  }

  std::string io::get_rendering_file_path()
  {
    return get_workspace_file_path("rendering.json");
  }

  std::string io::get_imgui_file_path()
  {
    return get_workspace_file_path("imgui.ini");
  }

  std::string io::get_render_output_file_path()
  {
    std::time_t t = std::time(nullptr);
    std::ostringstream oss;
    oss << "output_" << t << ".bmp";
    return get_renders_file_path(oss.str().c_str());
  }

  std::vector<std::string> io::discover_files(const std::string& path, const std::string& extension, bool include_extension)
  {
    std::vector<std::string> result;
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      if (entry.is_regular_file() && entry.path().extension() == extension)
      {
        std::string file_name = entry.path().filename().string();
        size_t index = file_name.find_last_of(".");
        result.push_back(file_name.substr(0, index));
      }
    }
    return result;
  }

  std::vector<std::string> io::discover_material_files(bool include_extension)
  {
    return discover_files(get_materials_dir(), ".json", include_extension);
  }

  std::vector<std::string> io::discover_texture_files(bool include_extension)
  {
    return discover_files(get_textures_dir(), ".json", include_extension);
  }

  std::vector<std::string> io::discover_mesh_files(bool include_extension)
  {
    return discover_files(get_meshes_dir(), ".json", include_extension);
  }
}