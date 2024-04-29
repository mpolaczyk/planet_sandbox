#pragma once

#include "nlohmann/json_fwd.hpp"

#include "core/core.h"
#include "object/object_visitor.h"

namespace engine
{
  class amaterial;
  class atexture;
  class astatic_mesh;
  class avertex_shader;
  class apixel_shader;
  class hhittable_base;
  class hscene;
  class hstatic_mesh;
  class hsphere;
  class hlight;
  
  class ENGINE_API serialize_object final : public fobject_visitor
  {
  public:
    serialize_object(nlohmann::json& json) : j(json){}
    
    virtual void visit(amaterial& object) const override;
    virtual void visit(atexture& object) const override;
    virtual void visit(astatic_mesh& object) const override;
    virtual void visit(avertex_shader& object) const override;
    virtual void visit(apixel_shader& object) const override;
    
    virtual void visit(hhittable_base& object) const override;
    virtual void visit(hscene& object) const override;
    virtual void visit(hstatic_mesh& object) const override;
    virtual void visit(hsphere& object) const override;
    virtual void visit(hlight& object) const override;
    
    nlohmann::json& j;
  };
  
  class ENGINE_API deserialize_object final : public fobject_visitor
  {
  public:
    deserialize_object(const nlohmann::json& json) : j(json){}
    
    virtual void visit(amaterial& object) const override;
    virtual void visit(atexture& object) const override;
    virtual void visit(astatic_mesh& object) const override;
    virtual void visit(avertex_shader& object) const override;
    virtual void visit(apixel_shader& object) const override;
    
    virtual void visit(hhittable_base& object) const override;
    virtual void visit(hscene& object) const override;
    virtual void visit(hstatic_mesh& object) const override;
    virtual void visit(hsphere& object) const override;
    virtual void visit(hlight& object) const override;
    
    const nlohmann::json& j;
  };
}
