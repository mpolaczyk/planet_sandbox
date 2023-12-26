#include "CppUnitTest.h"
#include <CppUnitTestAssert.h>

#include "core/application.h"

#include "renderers/cpu_renderer_preview.h"
#include "renderers/cpu_renderer_normals.h"
#include "renderers/cpu_renderer_faces.h"
#include "renderers/cpu_renderer_reference.h"

#include "engine/log.h"
#include "hittables/hittables.h"
#include "hittables/sphere.h"
#include "hittables/static_mesh.h"
#include "object/object_registry.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace engine;

application* engine::create_application()
{
  return new application();
}

namespace EngineTests
{
  TEST_CLASS(test_object)
  {
  public:
    static application* app;

    TEST_CLASS_INITIALIZE(test_init)
    {
      app = create_application();
      app->run();
    }

    TEST_CLASS_CLEANUP(test_cleanp)
    {
      delete app;
    }

    TEST_METHOD(object_find_type_from_string)
    {
      // Find class object by name
      std::string name = "static_mesh";
      const class_object* hco = REG.find_class(name);

      Assert::IsTrue(name == hco->get_class_name());
    }

    TEST_METHOD(object_spawn_from_class)
    {
      // Spawn object instance from class object that defines the type
      const class_object* hco = static_mesh::get_class_static();
      hittable* m1 = REG.spawn_from_class<hittable>(hco);

      Assert::IsTrue(hco == m1->get_class());
    }

    TEST_METHOD(object_types)
    {
      // A few checks for comparing class types and their hierachy
      renderer_base* b = renderer_base::spawn();
      renderer_base* r = cpu_renderer_reference::spawn();
      renderer_base* n = cpu_renderer_normals::spawn();

      // Spawn and no spawn
      Assert::IsNull(b, L"a");
      Assert::IsNotNull(r, L"aa");
      Assert::IsNotNull(n, L"aaa");

      // Basic class information
      Assert::IsTrue(r->get_class() == cpu_renderer_reference::get_class_static(), L"b");
      Assert::IsTrue(r->get_parent_class() == renderer_base::get_class_static(), L"bb");

      // Base class recognition
      Assert::IsTrue(r->is_child_of(object::get_class_static()), L"c");
      Assert::IsTrue(r->is_child_of(renderer_base::get_class_static()), L"cc");
      Assert::IsTrue(r->is_child_of(cpu_renderer_reference::get_class_static()), L"ccc");

      Assert::IsFalse(r->is_child_of(hittable::get_class_static()), L"d");
      Assert::IsFalse(r->is_child_of(cpu_renderer_normals::get_class_static()), L"dd");

      r->destroy();
      n->destroy();
    }


  };

  application* test_object::app = nullptr;
}
