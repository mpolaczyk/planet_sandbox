#pragma once

#include "nlohmann/json_fwd.hpp"

#include "core/core.h"
#include "object/object_visitor.h"

namespace engine
{
  struct ENGINE_API vserialize_object final : public vobject_visitor
  {
  public:
    vserialize_object(nlohmann::json& json) : j(json)
    {
    }

    virtual void visit_rrenderer_base(rrenderer_base& object) const;

    virtual void visit(amaterial& object) const override;
    virtual void visit(atexture& object) const override;
    virtual void visit(astatic_mesh& object) const override;
    virtual void visit(ashader& object) const override;

    virtual void visit(hhittable_base& object) const override;
    virtual void visit(hscene& object) const override;
    virtual void visit(hstatic_mesh& object) const override;
    virtual void visit(hsphere& object) const override;
    virtual void visit(hlight& object) const override;

    virtual void visit(rforward& object) const override;
    virtual void visit(rdeferred& object) const override;

    nlohmann::json& j;
  };

  struct ENGINE_API vdeserialize_object final : public vobject_visitor
  {
  public:
    vdeserialize_object(const nlohmann::json& json) : j(json)
    {
    }

    void visit_rrenderer_base(rrenderer_base& object) const;

    virtual void visit(amaterial& object) const override;
    virtual void visit(atexture& object) const override;
    virtual void visit(astatic_mesh& object) const override;
    virtual void visit(ashader& object) const override;

    virtual void visit(hhittable_base& object) const override; // TODO remove visitor from the base class
    virtual void visit(hscene& object) const override;
    virtual void visit(hstatic_mesh& object) const override;
    virtual void visit(hsphere& object) const override;
    virtual void visit(hlight& object) const override;

    virtual void visit(rforward& object) const override;
    virtual void visit(rdeferred& object) const override;

    const nlohmann::json& j;
  };
}
