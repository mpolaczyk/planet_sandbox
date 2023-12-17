#pragma once

#include <cassert>

#include "core/core.h"

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
