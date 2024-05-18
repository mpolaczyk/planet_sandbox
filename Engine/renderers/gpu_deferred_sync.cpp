#include <d3d11_1.h>
#include "renderer/dx11_lib.h"

#include "object/object_registry.h"
#include "object/object_visitor.h"
#include "gpu_deferred_sync.h"

#include "engine/log.h"
#include "hittables/light.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"

using namespace DirectX;

namespace engine
{
  OBJECT_DEFINE(rgpu_deferred_sync, rrenderer_base, GPU deferred sync)
  OBJECT_DEFINE_SPAWN(rgpu_deferred_sync)
  OBJECT_DEFINE_VISITOR(rgpu_deferred_sync)

  void rgpu_deferred_sync::init()
  {

  }

  void rgpu_deferred_sync::render_frame_impl()
  {

  }
}
