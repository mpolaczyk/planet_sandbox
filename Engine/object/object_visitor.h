#pragma once

#include <stdexcept>

#include "core/core.h"

// Visitor design pattern for managed object types
// Encapsulate operations as types and execute on objects so that no member or global functions have to be defined.
// Profit: No reason to bloat Engine classes with new members. They can be extended even in a separate project.
// This sounds cool but so far is pain in the ass.
// 1. Const correctness - some visitors may want to change the object. This requires both const and non-const flows.
// 2. Default implementation assumes that the visitor is abstract, but this enforces each child visitor to implement
//    dummy methods as some operations exist only for a subset of types.
// 3. Adding extra arguments to a child visitor objects can be done using the constructor but then it is applied to all
//    object types, which may not be desired.
// 4. Call performance. Don't use it for performance code as it traverses two polymorphic hierarchies.
namespace engine
{
  class amaterial;
  class atexture;
  class astatic_mesh;
  class ashader;
  class avertex_shader;
  class apixel_shader;
  class hhittable_base;
  class hscene;
  class hstatic_mesh;
  class hsphere;
  class hlight;
  class rrenderer_base;
  class rgpu_forward_sync;
  class rgpu_deferred_sync;

  struct ENGINE_API vobject_visitor
  {
  public:
    CTOR_DEFAULT(vobject_visitor)
    CTOR_MOVE_COPY_DELETE(vobject_visitor)
    VDTOR_DEFAULT(vobject_visitor)

    virtual void visit(amaterial& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(atexture& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(astatic_mesh& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(ashader& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(avertex_shader& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(apixel_shader& object) const { vobject_visitor::invalid_operation(); }

    virtual void visit(hhittable_base& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(hscene& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(hstatic_mesh& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(hsphere& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(hlight& object) const { vobject_visitor::invalid_operation(); }

    virtual void visit(rrenderer_base& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(rgpu_forward_sync& object) const { vobject_visitor::invalid_operation(); }
    virtual void visit(rgpu_deferred_sync& object) const { vobject_visitor::invalid_operation(); }

  private:
    static void invalid_operation()
    {
      throw std::runtime_error("Visitor operation not supported.");
    }
  };
}
