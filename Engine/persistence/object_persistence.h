#pragma once

#include "nlohmann/json_fwd.hpp"

#include "core/core.h"
#include "object/object_visitor.h"

namespace engine
{
  struct ENGINE_API vserialize_object final : public vobject_visitor
  {
  public:
    vserialize_object(nlohmann::json& json) : j(json){}
    
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
  
  struct ENGINE_API vdeserialize_object final : public vobject_visitor
  {
  public:
    vdeserialize_object(const nlohmann::json& json) : j(json){}
    
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
