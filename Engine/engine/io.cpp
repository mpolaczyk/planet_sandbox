#include <filesystem>
#include <fstream>
#include <sys/stat.h>

#include "engine/io.h"

namespace engine
{
  const char* fio::project_name;

  std::string fio::get_project_name()
  {
    return project_name;
  }

  void fio::init(const char* name)
  {
    project_name = name;
  }

  std::string fio::get_working_dir()
  {
    std::string current_dir = std::filesystem::current_path().string();
    std::ostringstream oss;
    oss << current_dir << "\\";
    return oss.str();
  }

  std::string fio::get_workspace_dir()
  {
    std::string working_dir = get_working_dir();
    std::ostringstream oss;
    oss << working_dir << "..\\..\\workspace\\";
    return oss.str();
  }

  std::string fio::get_project_dir()
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << project_name << "\\";
    return oss.str();
  }

  std::string fio::get_materials_dir()
  {
    std::string project_dir = get_project_dir();
    std::ostringstream oss;
    oss << project_dir << "Materials\\";
    return oss.str();
  }

  std::string fio::get_meshes_dir()
  {
    std::string project_dir = get_project_dir();
    std::ostringstream oss;
    oss << project_dir << "meshes\\";
    return oss.str();
  }

  std::string fio::get_textures_dir()
  {
    std::string project_dir = get_project_dir();
    std::ostringstream oss;
    oss << project_dir << "textures\\";
    return oss.str();
  }

  std::string fio::get_shaders_dir()
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << "shaders\\";
    return oss.str();
  }

  bool fio::validate_workspace_dir()
  {
    using namespace std;
    struct stat sb;
    if(stat(get_workspace_dir().c_str(), &sb) == 0)
    {
      return true;
    }
    return false;
  }

  std::string fio::get_workspace_file_path(const char* file_name)
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << file_name;
    return oss.str();
  }

  std::string fio::get_project_file_path(const char* file_name)
  {
    std::string project_dir = get_project_dir();
    std::ostringstream oss;
    oss << project_dir << file_name;
    return oss.str();
  }

  std::string fio::get_material_file_path(const char* file_name)
  {
    std::string materials_dir = get_materials_dir();
    std::ostringstream oss;
    oss << materials_dir << file_name;
    return oss.str();
  }

  std::string fio::get_mesh_file_path(const char* file_name)
  {
    std::string meshes_dir = get_meshes_dir();
    std::ostringstream oss;
    oss << meshes_dir << file_name;
    return oss.str();
  }

  std::string fio::get_texture_file_path(const char* file_name)
  {
    std::string textures_dir = get_textures_dir();
    std::ostringstream oss;
    oss << textures_dir << file_name;
    return oss.str();
  }

  std::string fio::get_shader_file_path(const char* file_name)
  {
    std::string shaders_dir = get_shaders_dir();
    std::ostringstream oss;
    oss << shaders_dir << file_name;
    return oss.str();
  }

  std::string fio::get_window_file_path()
  {
    return get_workspace_file_path("window.json");
  }

  std::string fio::get_scene_file_path()
  {
    return get_project_file_path("scene.json");
  }

  std::string fio::get_imgui_file_path()
  {
    return get_workspace_file_path("imgui.ini");
  }

  std::vector<std::string> fio::discover_files(const std::string& path, const std::string& extension, bool include_extension)
  {
    std::vector<std::string> result;
    for(const auto& entry : std::filesystem::directory_iterator(path))
    {
      if(entry.is_regular_file() && entry.path().extension() == extension)
      {
        std::string file_name = entry.path().filename().string();
        size_t index = file_name.find_last_of(".");
        result.push_back(file_name.substr(0, index));
      }
    }
    return result;
  }

  std::vector<std::string> fio::discover_material_files(bool include_extension)
  {
    return discover_files(get_materials_dir(), ".material", include_extension);
  }

  std::vector<std::string> fio::discover_texture_files(bool include_extension)
  {
    return discover_files(get_textures_dir(), ".texture", include_extension);
  }

  std::vector<std::string> fio::discover_mesh_files(bool include_extension)
  {
    return discover_files(get_meshes_dir(), ".mesh", include_extension);
  }

  std::vector<std::string> fio::discover_pixel_shader_files(bool include_extension)
  {
    return discover_files(get_shaders_dir(), ".pixel_shader", include_extension);
  }
  
  std::vector<std::string> fio::discover_vertex_shader_files(bool include_extension)
  {
    return discover_files(get_shaders_dir(), ".vertex_shader", include_extension);
  }
}
