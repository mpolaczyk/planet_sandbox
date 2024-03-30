#pragma once
#include "ui.h"
#include "object/object_visitor.h"

namespace ray_tracer
{
  class fdraw_edit_panel final : public fobject_visitor
  {
  public:
    virtual void visit(class hhittable_base& object) const override;
    virtual void visit(class hscene& object) const override;
    virtual void visit(class hstatic_mesh& object) const override;
    virtual void visit(class hsphere& object) const override;

    virtual void visit(class amaterial& object) const override;

  private:
    fselection_combo_model<amaterial> m_model;  // Used by hittable
  };
}