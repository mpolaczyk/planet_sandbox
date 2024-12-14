#include "object/object_registry_temp.cpp"

#include "asset/asset.h"
#include "assets/mesh.h"
#include "assets/material.h"
#include "assets/texture.h"
#include "assets/shader.h"
#include "assets/vertex_shader.h"
#include "assets/pixel_shader.h"
#include "renderer/renderer_base.h"
#include "renderers/forward.h"
#include "renderers/deferred.h"
#include "hittables/scene.h"
#include "hittables/sphere.h"
#include "hittables/static_mesh.h"
#include "hittables/light.h"
#include "object/object.h"

#define OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(CLASS_NAME) \
  template ENGINE_API bool fobject_registry::add<CLASS_NAME>(CLASS_NAME* instance); \
  template ENGINE_API CLASS_NAME* fobject_registry::get<CLASS_NAME>(int id) const; \
  template ENGINE_API std::vector<CLASS_NAME*> fobject_registry::get_all_by_type<CLASS_NAME>(); \
  template ENGINE_API std::vector<const CLASS_NAME*> fobject_registry::get_all_by_type<const CLASS_NAME>(); \
  template ENGINE_API CLASS_NAME* fobject_registry::find<CLASS_NAME>(std::function<bool(CLASS_NAME*)> predicate) const; \
  template ENGINE_API const CLASS_NAME* fobject_registry::find<const CLASS_NAME>(std::function<bool(const CLASS_NAME*)> predicate) const; \
  template ENGINE_API std::vector<CLASS_NAME*> fobject_registry::find_all<CLASS_NAME>(std::function<bool(CLASS_NAME*)> predicate) const; \
  template ENGINE_API std::vector<const CLASS_NAME*> fobject_registry::find_all<const CLASS_NAME>(std::function<bool(const CLASS_NAME*)> predicate) const; \
  template ENGINE_API CLASS_NAME* fobject_registry::spawn_from_class(const oclass_object* type);

#define OBJECT_REGISTRY_EXPLICIT_INSTANTIATE_COPY(CLASS_NAME) \
  template ENGINE_API CLASS_NAME* fobject_registry::copy_shallow<CLASS_NAME>(const CLASS_NAME* source);

// Creates an instance of a class object for a given object type
#define CLASS_OBJECT_REGISTER(CLASS_NAME, PARENT_CLASS_NAME) \
  { \
    register_class(#CLASS_NAME, #PARENT_CLASS_NAME, []() -> oobject* { return CLASS_NAME::spawn(); }); \
  }

// FIX: This content should be automatically generated by the pre-compile script based on all the types defined in the code base
namespace engine
{
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(oclass_object)

  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(aasset_base)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(astatic_mesh)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(atexture)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(amaterial)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(ashader)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(avertex_shader)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(apixel_shader)

  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(rrenderer_base)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(rforward)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(rdeferred)

  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(hhittable_base)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(hscene)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(hsphere)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(hstatic_mesh)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE(hlight)

  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE_COPY(aasset_base)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE_COPY(ashader)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE_COPY(astatic_mesh)

  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE_COPY(hhittable_base)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE_COPY(hscene)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE_COPY(hsphere)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE_COPY(hstatic_mesh)
  OBJECT_REGISTRY_EXPLICIT_INSTANTIATE_COPY(hlight)

  void fobject_registry::create_class_objects()
  {
    CLASS_OBJECT_REGISTER(oobject, oobject)
    CLASS_OBJECT_REGISTER(oclass_object, oobject)

    CLASS_OBJECT_REGISTER(aasset_base, oobject)
    CLASS_OBJECT_REGISTER(amaterial, aasset_base)
    CLASS_OBJECT_REGISTER(atexture, aasset_base)
    CLASS_OBJECT_REGISTER(astatic_mesh, aasset_base)
    CLASS_OBJECT_REGISTER(ashader, aasset_base)
    CLASS_OBJECT_REGISTER(avertex_shader, ashader)
    CLASS_OBJECT_REGISTER(apixel_shader, ashader)

    CLASS_OBJECT_REGISTER(rrenderer_base, oobject)
    CLASS_OBJECT_REGISTER(rforward, rrenderer_base)
    CLASS_OBJECT_REGISTER(rdeferred, rrenderer_base)

    CLASS_OBJECT_REGISTER(hhittable_base, oobject)
    CLASS_OBJECT_REGISTER(hscene, hhittable_base)
    CLASS_OBJECT_REGISTER(hsphere, hhittable_base)
    CLASS_OBJECT_REGISTER(hstatic_mesh, hhittable_base)
    CLASS_OBJECT_REGISTER(hlight, hhittable_base)
  }
}

#undef OBJECT_REGISTRY_EXPLICIT_INSTANTIATE
#undef CLASS_OBJECT_EXPLICIT_INSTANTIATE
#undef CLASS_OBJECT_REGISTER
