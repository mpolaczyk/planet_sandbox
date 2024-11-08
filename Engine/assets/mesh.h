#pragma once

#include <directxcollision.h>

#include "core/core.h"
#include "asset/asset.h"
#include "math/vertex_data.h"
#include "renderer/gpu_resources.h"

namespace engine
{
  class ENGINE_API astatic_mesh : public aasset_base
  {
  public:
    OBJECT_DECLARE(astatic_mesh, aasset_base)
    OBJECT_DECLARE_VISITOR

    virtual std::string get_extension() const override;
    virtual std::string get_folder() const override;
    virtual bool load(const std::string& name) override;
    virtual void save() override;
    
    // JSON persistent
    std::string obj_file_name;

    // Runtime state
    DirectX::BoundingBox bounding_box;
    std::vector<fvertex_data> vertex_list;
    std::vector<fface_data> face_list;
    
    bool is_resource_online = false;
    fstatic_mesh_resource resource;
  };
}
