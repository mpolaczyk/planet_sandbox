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
  class avertex_shader;
  class apixel_shader;
  class hhittable_base;
  class hscene;
  class hstatic_mesh;
  class hsphere;
  class hlight;
  
  struct ENGINE_API vobject_visitor
  {
  public:
    vobject_visitor() = default;
    virtual ~vobject_visitor() = default;
    vobject_visitor(const vobject_visitor&) = delete;
    vobject_visitor(vobject_visitor&&) = delete;
    vobject_visitor& operator=(vobject_visitor&&) = delete;
    vobject_visitor& operator=(const vobject_visitor&) = delete;

    virtual void visit(amaterial& object) const       { vobject_visitor::invalid_operation(); }
    virtual void visit(atexture& object) const        { vobject_visitor::invalid_operation(); }
    virtual void visit(astatic_mesh& object) const    { vobject_visitor::invalid_operation(); }
    virtual void visit(avertex_shader& object) const  { vobject_visitor::invalid_operation(); }
    virtual void visit(apixel_shader& object) const   { vobject_visitor::invalid_operation(); }
    
    virtual void visit(hhittable_base& object) const  { vobject_visitor::invalid_operation(); }
    virtual void visit(hscene& object) const          { vobject_visitor::invalid_operation(); }
    virtual void visit(hstatic_mesh& object) const    { vobject_visitor::invalid_operation(); }
    virtual void visit(hsphere& object) const         { vobject_visitor::invalid_operation(); }
    virtual void visit(hlight& object) const          { vobject_visitor::invalid_operation(); }

  private:
    static void invalid_operation()
    {
      throw std::runtime_error("Visitor operation not supported.");
    }
  };    
}
