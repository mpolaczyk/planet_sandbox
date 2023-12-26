#pragma once
#include "ui.h"
#include "object/object_visitor.h"

namespace ray_tracer
{
  class draw_edit_panel final : public object_visitor
  {
  public:
    virtual void visit(class hittable& object) const override;
    virtual void visit(class scene& object) const override;
    virtual void visit(class static_mesh& object) const override;
    virtual void visit(class sphere& object) const override;

    virtual void visit(class material_asset& object) const override;

  private:
    selection_combo_model<material_asset> m_model;  // Used by hittable
  };
}