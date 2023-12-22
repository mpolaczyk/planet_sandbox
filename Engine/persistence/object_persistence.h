#pragma once

#include "nlohmann/json_fwd.hpp"

#include "core/core.h"
#include "object/object_visitor.h"

namespace engine
{
  class ENGINE_API serialize_object final : public object_visitor
  {
  public:
    serialize_object(nlohmann::json& json) : j(json){}
    
    virtual void visit(class material_asset& object) const override;
    virtual void visit(class texture_asset& object) const override;
    virtual void visit(class static_mesh_asset& object) const override;
    virtual void visit(class vertex_shader_asset& object) const override;
    virtual void visit(class pixel_shader_asset& object) const override;
    
    virtual void visit(class hittable& object) const override;
    virtual void visit(class scene& object) const override;
    virtual void visit(class static_mesh& object) const override;
    virtual void visit(class sphere& object) const override;
    
    nlohmann::json& j;
  };
  
  class ENGINE_API deserialize_object final : public object_visitor
  {
  public:
    deserialize_object(const nlohmann::json& json) : j(json){}
    
    virtual void visit(class material_asset& object) const override;
    virtual void visit(class texture_asset& object) const override;
    virtual void visit(class static_mesh_asset& object) const override;
    virtual void visit(class vertex_shader_asset& object) const override;
    virtual void visit(class pixel_shader_asset& object) const override;
    
    virtual void visit(class hittable& object) const override;
    virtual void visit(class scene& object) const override;
    virtual void visit(class static_mesh& object) const override;
    virtual void visit(class sphere& object) const override;
    
    const nlohmann::json& j;
  };
}
