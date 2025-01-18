#include "stdafx.h"

#include <sys/stat.h>

#include "engine/io.h"
#include "engine/time.h"

namespace engine
{
  std::string fio::project_name;
  std::string fio::mesh_extension = ".mesh";
  std::string fio::material_extension = ".material";
  std::string fio::texture_extension = ".texture";
  std::string fio::pixel_shader_extension = ".pixel_shader";
  std::string fio::vertex_shader_extension = ".vertex_shader";
  std::string fio::ray_tracing_shader_extension = ".ray_tracing_shader";
  std::string fio::working_dir;
  std::string fio::workspace_dir;
  std::string fio::project_dir;
  std::string fio::materials_dir;
  std::string fio::meshes_dir;
  std::string fio::textures_dir;
  std::string fio::shaders_dir;

  std::string fio::get_project_name()
  {
    return project_name;
  }

  void fio::init(const std::string& name)
  {
    project_name = name;
    {
      const std::string current_dir = std::filesystem::current_path().string();
      std::ostringstream oss;
      oss << current_dir << "\\";
      working_dir = oss.str();
    }
    {
      std::ostringstream oss;
      oss << working_dir << "..\\..\\workspace\\";
      workspace_dir = oss.str();
    }
    {
      std::ostringstream oss;
      oss << workspace_dir << project_name << "\\";
      project_dir = oss.str();
    }
    {
      std::ostringstream oss;
      oss << project_dir << "materials\\";
      materials_dir = oss.str();
    }
    {
      std::ostringstream oss;
      oss << project_dir << "meshes\\";
      meshes_dir = oss.str();
    }
    {
      std::ostringstream oss;
      oss << project_dir << "textures\\";
      textures_dir = oss.str();
    }
    {
      std::ostringstream oss;
      oss << workspace_dir << "shaders\\";
      shaders_dir = oss.str();
    }
  }

  std::string fio::get_mesh_extension()
  {
    return mesh_extension;
  }
  
  std::string fio::get_material_extension()
  {
    return material_extension;
  }
  
  std::string fio::get_texture_extension()
  {
    return texture_extension;
  }
  
  std::string fio::get_pixel_shader_extension()
  {
    return pixel_shader_extension;
  }
  
  std::string fio::get_vertex_shader_extension()
  {
    return vertex_shader_extension;
  }

  std::string fio::get_ray_tracing_shader_extension()
  {
    return ray_tracing_shader_extension;
  }
  
  std::string fio::get_working_dir()
  {
    return working_dir;
  }

  std::string fio::get_workspace_dir()
  {
    return workspace_dir;
  }

  std::string fio::get_project_dir()
  {
    return project_dir;
  }

  std::string fio::get_materials_dir()
  {
    return materials_dir;
  }

  std::string fio::get_meshes_dir()
  {
    return meshes_dir;
  }

  std::string fio::get_textures_dir()
  {
    return textures_dir;
  }

  std::string fio::get_shaders_dir()
  {
    return shaders_dir;
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
    return discover_files(get_materials_dir(), get_material_extension(), include_extension);
  }

  std::vector<std::string> fio::discover_texture_files(bool include_extension)
  {
    return discover_files(get_textures_dir(), get_texture_extension(), include_extension);
  }

  std::vector<std::string> fio::discover_mesh_files(bool include_extension)
  {
    return discover_files(get_meshes_dir(), get_mesh_extension(), include_extension);
  }

  std::vector<std::string> fio::discover_pixel_shader_files(bool include_extension)
  {
    return discover_files(get_shaders_dir(), get_pixel_shader_extension(), include_extension);
  }
  
  std::vector<std::string> fio::discover_vertex_shader_files(bool include_extension)
  {
    return discover_files(get_shaders_dir(), get_vertex_shader_extension(), include_extension);
  }

  int64_t fio::get_last_write_time(const char* file_path)
  {
    return ftime::get_file_write_time(file_path);
  }
}
