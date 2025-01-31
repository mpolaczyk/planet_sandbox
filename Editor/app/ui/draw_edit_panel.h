﻿#pragma once

#include "core/rtti/object_visitor.h"

namespace engine
{
  class hhittable_base;
  class hscene;
  class hstatic_mesh;
  class hlight;
  class amaterial;
  class rrenderer_base;
  class rforward;
  class rdeferred;
  class rray_tracing;
}

namespace editor
{
  struct vdraw_edit_panel final : public vobject_visitor
  {
  public:
    void visit_hhittable_base(hhittable_base& object) const;
    void visit_aasset_base(aasset_base& object) const;
    void visit_rrenderer_base(rrenderer_base& object) const;

    virtual void visit(hstatic_mesh& object) const override;
    virtual void visit(hlight& object) const override;
    virtual void visit(amaterial& object) const override;
    virtual void visit(rforward& object) const override;
    virtual void visit(rdeferred& object) const override;
    virtual void visit(rray_tracing& object) const override;
  };
}