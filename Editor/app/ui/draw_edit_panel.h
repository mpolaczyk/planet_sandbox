#pragma once

#include "ui.h"
#include "object/object_visitor.h"

namespace engine
{
  class hhittable_base;
  class hscene;
  class hstatic_mesh;
  class hsphere;
  class hlight;
  class amaterial;
}

namespace editor
{
  struct vdraw_edit_panel final : public vobject_visitor
  {
  public:
    virtual void visit(hhittable_base& object) const override;
    virtual void visit(hscene& object) const override;
    virtual void visit(hstatic_mesh& object) const override;
    virtual void visit(hsphere& object) const override;
    virtual void visit(hlight& object) const override;

    virtual void visit(amaterial& object) const override;

  private:
    fselection_combo_model<amaterial> m_model;  // Used by hittable
  };
}