#include "CppUnitTest.h"
#include <CppUnitTestAssert.h>

#include "core/application.h"

#include "object/factories.h"
#include "object/object_types.h"

#include "renderers/cpu_renderer_preview.h"
#include "renderers/cpu_renderer_normals.h"
#include "renderers/cpu_renderer_faces.h"
#include "renderers/cpu_renderer_reference.h"

#include "engine/log.h"
#include "hittables/hittables.h"
#include "hittables/sphere.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace engine;

application* engine::create_appliation()
{
  return new application();
}

namespace EngineTests
{
  TEST_CLASS(test_rtti)
  {
  public:
    static application* app;

    TEST_CLASS_INITIALIZE(test_init)
    {
      app = create_appliation();
      app->run();
    }

    TEST_CLASS_CLEANUP(test_cleanp)
    {
      delete app;
    }

    TEST_METHOD(rtti_static)
    {
      cpu_renderer_base* o = object_factory::spawn_renderer(object_type::object);

      // No spawn
      Assert::IsNull(o);

      // Static checks, object is its own parent
      Assert::IsTrue(object::get_type_static() == object_type::object, L"a");
      Assert::IsTrue(object::get_parent_type_static() == object_type::object, L"aa");
      Assert::IsTrue(object::is_type_static(object_type::object), L"aaa");

      // Is type is inclusive
      Assert::IsTrue(hittable::is_type_static(object_type::hittable), L"d");
      Assert::IsTrue(sphere::is_type_static(object_type::hittable), L"dd");
      Assert::IsTrue(sphere::is_type_static(object_type::object), L"ddd");

      // Types outside of the inheritance tree
      Assert::IsFalse(object::get_type_static() == object_type::asset_base, L"b");
      Assert::IsFalse(object::get_parent_type_static() == object_type::asset_base, L"bb");
      Assert::IsFalse(object::is_type_static(object_type::asset_base), L"bbb");
      Assert::IsFalse(asset_base::is_type_static(object_type::hittable), L"bbbb");
    }

    TEST_METHOD(rtti_instances)
    {
      cpu_renderer_base* b = object_factory::spawn_renderer(object_type::cpu_renderer_base);
      cpu_renderer_base* r = object_factory::spawn_renderer(object_type::cpu_renderer_reference);
      cpu_renderer_base* n = object_factory::spawn_renderer(object_type::cpu_renderer_normals);

      // Spawn and no spawn
      Assert::IsNull(b, L"a");
      Assert::IsNotNull(r, L"aa");
      Assert::IsNotNull(n, L"aaa");

      // Basic class information
      Assert::IsTrue(r->get_type() == object_type::cpu_renderer_reference, L"b");
      Assert::IsTrue(r->get_parent_type() == object_type::cpu_renderer_base, L"bb");

      // Base class recognition
      Assert::IsTrue(r->is_type(object_type::object), L"c");
      Assert::IsTrue(r->is_type(object_type::cpu_renderer_base), L"cc");
      Assert::IsTrue(r->is_type(object_type::cpu_renderer_reference), L"ccc");

      Assert::IsFalse(r->is_type(object_type::hittable), L"d");
      Assert::IsFalse(r->is_type(object_type::cpu_renderer_normals), L"dd");

      r->destroy();
      n->destroy();
    }
  };

  application* test_rtti::app = nullptr;
}
