#pragma once

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

    // FIX no operations defined yet, try with drawing
    // serialize and deserialize don't fit the pattern due to additional arguments, use varargs?
    
    virtual void visit(class object const& object) const = 0;
  };    
}
