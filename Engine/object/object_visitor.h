#pragma once

#include <cassert>

#include "core/core.h"

// Visitor design pattern for managed object types
// Encapsulate operations as types and execute on objects so that no member or global functions have to be defined.
// This sounds cool but so far is pain in the ass.
// 1. Const correctness - some visitors may want to change the object. This requires both const and non-const flows.
// 2. Default implementation assumes that the visitor is abstract, but this enforces each child visitor to implement
//    dummy methods as some operations exist only for a subset of types.
// 3. Adding extra arguments to a child visitor objects can be done using the constructor but then it is applied to all
//    object types, which may not be desired.
// 4. Call performance. Don't use it for performance code as it traverses two polymorphic hierarchies.
namespace engine
{
  class ENGINE_API fobject_visitor
  {
  public:
    fobject_visitor() = default;
    virtual ~fobject_visitor() = default;
    fobject_visitor(const fobject_visitor&) = delete;
    fobject_visitor(fobject_visitor&&) = delete;
    fobject_visitor& operator=(fobject_visitor&&) = delete;
    fobject_visitor& operator=(const fobject_visitor&) = delete;

    virtual void visit(class amaterial& object) const { assert(false); }
    virtual void visit(class atexture& object) const { assert(false); }
    virtual void visit(class astatic_mesh& object) const { assert(false); }
    virtual void visit(class avertex_shader& object) const { assert(false); }
    virtual void visit(class apixel_shader& object) const { assert(false); }
    
    virtual void visit(class hhittable_base& object) const { assert(false); }
    virtual void visit(class hscene& object) const { assert(false); }
    virtual void visit(class hstatic_mesh& object) const { assert(false); }
    virtual void visit(class hsphere& object) const { assert(false); }
    virtual void visit(class hlight& object) const { assert(false); }
  };    
}
