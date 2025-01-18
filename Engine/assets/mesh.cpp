#include "stdafx.h"

#include "assets/mesh.h"

#include "engine/io.h"
#include "engine/resources/resources_io.h"
#include "engine/persistence/object_persistence.h"

namespace engine
{
  OBJECT_DEFINE(astatic_mesh, aasset_base, Static mesh asset)
  OBJECT_DEFINE_SPAWN(astatic_mesh)
  OBJECT_DEFINE_VISITOR(astatic_mesh)

  std::string astatic_mesh::get_extension() const
  {
    return fio::get_mesh_extension();
  }
  
  std::string astatic_mesh::get_folder() const
  {
    return fio::get_meshes_dir();
  }
  
  bool astatic_mesh::load(const std::string& name)
  {
    aasset_base::load(name);

    LOG_DEBUG("Loading mesh: {0}", name);

    std::ostringstream oss;
    oss << name << fio::get_mesh_extension();
    const std::string file_path = fio::get_mesh_file_path(oss.str().c_str());
    std::ifstream input_stream(file_path.c_str());
    if(input_stream.fail())
    {
      LOG_ERROR("Unable to open mesh: {0}", file_path);
      return false;
    }

    nlohmann::json j;
    input_stream >> j;
    accept(vdeserialize_object(j));
    set_display_name(name);

    if(!load_obj(obj_file_name, this))
    {
      LOG_ERROR("Failed to load mesh: {0}", obj_file_name);
      return false;
    }
    return true;
  }

  void astatic_mesh::save()
  {
    nlohmann::json j;
    accept(vserialize_object(j));

    std::ostringstream oss;
    oss << name << fio::get_mesh_extension();
    std::ofstream o(fio::get_mesh_file_path(oss.str().c_str()), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if(o.is_open())
    {
      o.write(str.data(), str.length());
    }
    else
    {
      LOG_ERROR("Unable to save mesh: {0}", oss.str());
    }
    o.close();
  }
}
