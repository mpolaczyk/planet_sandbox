#include "stdafx.h"

#include "forward.h"

#include "passes/debug_pass.h"
#include "passes/forward_pass.h"

#include "engine/renderer/command_list.h"

namespace engine
{
  // Because funique_ptr<forward declared type> requires destructor where the type is complete
  rforward::rforward() = default;
  rforward::~rforward() = default;

  OBJECT_DEFINE(rforward, rrenderer_base, Forward renderer)
  OBJECT_DEFINE_SPAWN(rforward)
  OBJECT_DEFINE_VISITOR(rforward)
  
  bool rforward::init_passes()
  {
    if (!forward_pass)
    {
      forward_pass.reset(new fforward_pass);
    }
    if (!debug_pass)
    {
      debug_pass.reset(new fdebug_pass);
    }
    return forward_pass->init(&context) && debug_pass->init(&context);
  }

  void rforward::draw_internal(fcommand_list* command_list)
  {
    forward_pass->draw(&context, command_list);

    debug_pass->blend_on = &forward_pass->color;
    
    debug_pass->draw(&context, command_list);
  }

  ftexture_resource* rforward::get_color()
  {
    return &forward_pass->color;
  }

  ftexture_resource* rforward::get_depth()
  {
    return &forward_pass->depth;
  }
}