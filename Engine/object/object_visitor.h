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
  class ENGINE_API object_visitor
  {
  public:
    object_visitor() = default;
    virtual ~object_visitor() = default;
    object_visitor(const object_visitor&) = delete;
    object_visitor(object_visitor&&) = delete;
    object_visitor& operator=(object_visitor&&) = delete;
    object_visitor& operator=(const object_visitor&) = delete;

    virtual void visit(class material_asset& object) const { assert(false); }
    virtual void visit(class texture_asset& object) const { assert(false); }
    virtual void visit(class static_mesh_asset& object) const { assert(false); }
    
    virtual void visit(class hittable& object) const { assert(false); }
    virtual void visit(class scene& object) const { assert(false); }
    virtual void visit(class static_mesh& object) const { assert(false); }
    virtual void visit(class sphere& object) const { assert(false); }
  };    
}
