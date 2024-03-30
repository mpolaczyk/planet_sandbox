#pragma once

#include "nlohmann/json_fwd.hpp"

#include "core/core.h"
#include "object/object_visitor.h"

namespace engine
{
  class ENGINE_API serialize_object final : public fobject_visitor
  {
  public:
    serialize_object(nlohmann::json& json) : j(json){}
    
    virtual void visit(class amaterial& object) const override;
    virtual void visit(class atexture& object) const override;
    virtual void visit(class astatic_mesh& object) const override;
    virtual void visit(class avertex_shader& object) const override;
    virtual void visit(class apixel_shader& object) const override;
    
    virtual void visit(class hhittable_base& object) const override;
    virtual void visit(class hscene& object) const override;
    virtual void visit(class hstatic_mesh& object) const override;
    virtual void visit(class hsphere& object) const override;
    
    nlohmann::json& j;
  };
  
  class ENGINE_API deserialize_object final : public fobject_visitor
  {
  public:
    deserialize_object(const nlohmann::json& json) : j(json){}
    
    virtual void visit(class amaterial& object) const override;
    virtual void visit(class atexture& object) const override;
    virtual void visit(class astatic_mesh& object) const override;
    virtual void visit(class avertex_shader& object) const override;
    virtual void visit(class apixel_shader& object) const override;
    
    virtual void visit(class hhittable_base& object) const override;
    virtual void visit(class hscene& object) const override;
    virtual void visit(class hstatic_mesh& object) const override;
    virtual void visit(class hsphere& object) const override;
    
    const nlohmann::json& j;
  };
}
