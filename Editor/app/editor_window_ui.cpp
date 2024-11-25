#include "stdafx.h"

#include <stdio.h>

#include "core/windows_minimal.h"

#include "ui/draw_edit_panel.h"
#include "imgui.h"

#include "math/chunk_generator.h"
#include "renderer/renderer_base.h"
#include "math/camera.h"

#include "core/core.h"
#include "engine.h"
#include "app/editor_app.h"
#include "app/editor_window.h"
#include "engine/string_tools.h"
#include "hittables/hittables.h"
#include "hittables/static_mesh.h"
#include "hittables/scene.h"
#include "resources/ffbx.h"

#include "reactphysics3d/engine/PhysicsWorld.h"
#include "reactphysics3d/mathematics/Ray.h"
#include "reactphysics3d/collision/RaycastInfo.h"

namespace editor
{
  void feditor_window::draw_editor_window(feditor_window_model& model)
  {
    ImGui::Begin("EDITOR", nullptr);
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "STATS");
    ImGui::Separator();
    ImGui::Text("Update %.3f ms", fapplication::stat_update_time.get_last_time_ms());
    ImGui::Text("Draw %.3f ms", fapplication::stat_draw_time.get_last_time_ms());
    ImGui::Text("Render %.3f ms", fapplication::stat_render_time.get_last_time_ms());
    float frame_time = fapplication::stat_frame_time.get_last_time_ms();
    float fps = 1.0f/frame_time*1000.0f;
    ImGui::Text("Frame %.3f ms  %.2f FPS", frame_time, fps);
    draw_hotkeys_panel();
    draw_materials_panel(model.rp_model);
    draw_object_registry_panel();
    ImGui::End();
  }
  void feditor_window::draw_hotkeys_panel()
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "HOTKEYS");
    ImGui::Separator();
    ImGui::Text("LMB (on image) - select object");
    ImGui::Text("Scroll - Camera speed (current speed: %f)", get_editor_app()->scene_root->camera.move_speed);
    ImGui::Text("QWEASD - Camera movement");
    ImGui::Text("RMB - Camera rotation");
    ImGui::Text("ZXC + mouse - Object movement");
    ImGui::Text("Space - Toggle simulation");
  }
  void feditor_window::draw_materials_panel(fmaterials_panel_model& model)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "MATERIALS");
    ImGui::Separator();
    if (ImGui::MenuItem("SAVE ALL"))
    {
      feditor_app::save_materials();
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SAVED!");
    }
    ImGui::Separator();

    model.m_model.objects = REG.get_all_by_type<const amaterial>();
    fui_helper::draw_selection_combo<amaterial>(model.m_model, "Material", [=](const amaterial* obj) -> bool{ return true; }, nullptr);
    if (model.m_model.selected_object != nullptr)
    {
      const_cast<amaterial*>(model.m_model.selected_object)->accept(vdraw_edit_panel());
    }
  }
  void feditor_window::draw_object_registry_panel()
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OBJECT REGISTRY");
    ImGui::Separator();

    const std::vector<oobject*>& objects = REG.get_all();

    int num_objects = (int)objects.size();
    if (ImGui::BeginListBox("Assets", ImVec2(-FLT_MIN, 25 * ImGui::GetTextLineHeightWithSpacing())))
    {
      for (int n = 0; n < num_objects; n++)
      {
        std::ostringstream oss;
        const oobject* obj = objects[n];
        oss << obj->get_runtime_id() << ":" << obj->get_class()->get_class_name() << "      " << obj->get_display_name();
        if (ImGui::Selectable(oss.str().c_str())) {}
      }
      ImGui::EndListBox();
    }
  }

  void feditor_window::draw_scene_window(fscene_window_model& model)
  {
    ImGui::Begin("SCENE", nullptr);
    if (ImGui::MenuItem("SAVE SCENE"))
    {
      fapplication::get_instance()->save_scene_state();
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SAVED!");
    }
    ImGui::Separator();

    if (ImGui::MenuItem("LOAD FROM FBX"))
    {
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Importing!");
      engine::ffbx::load_fbx_assimp(model.import_file, get_editor_app()->scene_root);
    }
    fui_helper::input_text("FBX file", model.import_file);

    draw_renderer_panel(model.rp_model);
    draw_camera_panel();
    draw_scene_panel();
    draw_scene_objects_panel(model.op_model);
    ImGui::End();
  }
  void feditor_window::draw_renderer_panel(frenderer_panel_model& model)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "RENDERER");
    ImGui::Separator();

    // Get list of all renderer classes
    // FIX this does not work for class objects!
    // co->is_child_of(renderer_base::get_class_static());
    model.r_model.objects = REG.find_all<const oclass_object>([](const oclass_object* co) -> bool{
      return co->get_parent_class_name() == rrenderer_base::get_class_static()->get_class_name();
    });

    // Renderer class selection combo box
    rrenderer_base* renderer = get_editor_app()->scene_root->renderer;
    const oclass_object* renderer_class = renderer->get_class();
    fui_helper::draw_selection_combo<oclass_object>(model.r_model, "Renderer class", [=](const oclass_object* obj) -> bool { return true; }, renderer_class);
    if(model.r_model.selected_object != renderer_class)
    {
      // Different renderer is chosen, find instance or create new one if none exists
      renderer = REG.find<rrenderer_base>([=](const rrenderer_base* obj) -> bool { return obj->get_class() == model.r_model.selected_object; });
      if(renderer == nullptr)
      {
        renderer = REG.spawn_from_class<rrenderer_base>(model.r_model.selected_object);
      }
      get_editor_app()->scene_root->renderer = renderer;
    }
    ImGui::Separator();

    // Renderer edit panel
    get_editor_app()->scene_root->renderer->accept(vdraw_edit_panel());
    ImGui::Separator();

    if (get_editor_app()->scene_root->renderer == nullptr)
    {
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "No renderer active");
    }
  }
  void feditor_window::draw_camera_panel()
  {
    fcamera& camera = get_editor_app()->scene_root->camera;
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CAMERA");
    ImGui::Separator();
    ImGui::InputFloat("Field of view", &camera.field_of_view, 1.0f, 189.0f, "%.0f");
    ImGui::Separator();
    ImGui::InputFloat3("Look from", camera.location.e, "%.2f");
    ImGui::InputFloat("Pitch", &camera.pitch, 0.1f, 1.0f, "%.2f");
    ImGui::InputFloat("Yaw", &camera.yaw, 0.1f, 1.0f, "%.2f");
    ImGui::Separator();
  }
  void feditor_window::draw_scene_panel()
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SCENE");
    ImGui::Separator();

    DirectX::XMFLOAT4& temp = get_editor_app()->scene_root->ambient_light_color;
    float temp_arr[4] = { temp.x,temp.y,temp.z,temp.w };
    ImGui::ColorEdit4("Ambient", temp_arr, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
    temp = { temp_arr[0],temp_arr[1],temp_arr[2],temp_arr[3] };

    if(get_editor_app()->wants_to_simulate_physics)
    {
      if(ImGui::Button("Reset simulation"))
      {
        get_editor_app()->wants_to_simulate_physics = false;
      }
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "SIMULATING!");
    }
    else
    {
      if(ImGui::Button("Start simulation"))
      {
        get_editor_app()->wants_to_simulate_physics = true;
      }      
    }
  }
  void feditor_window::draw_scene_objects_panel(fobjects_panel_model& model)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SCENE OBJECTS");
    ImGui::Separator();

    draw_new_object_panel(model.nop_model);
    ImGui::SameLine();
    draw_delete_object_panel(model.d_model);

    fui_helper::input_text("Name filter", model.object_name_filter);

    if(selected_object == nullptr)
    {
      model.selected_id = -1;
    }
    int num_objects = (int)get_editor_app()->scene_root->objects.size();
    if (ImGui::BeginListBox("Objects", ImVec2(-FLT_MIN, fmath::min1(10, (float)num_objects + 1) * ImGui::GetTextLineHeightWithSpacing())))
    {
      for (int n = 0; n < num_objects; n++)
      {
        hhittable_base* obj = get_editor_app()->scene_root->objects[n];
        if (selected_object != nullptr && obj == selected_object)
        {
          model.m_model.selected_id = -1;
          model.selected_id = n;
          model.d_model.selected_id = n;
        }
        std::string obj_name = obj->get_display_name();
        std::ostringstream oss;
        oss << obj_name;
        if (!model.object_name_filter.empty() && !fstring_tools::contains(obj_name, model.object_name_filter))
        {
          continue;
        }
        if (ImGui::Selectable(oss.str().c_str(), model.selected_id == n))
        {
          model.m_model.reset();
          model.selected_id = n;
          model.d_model.selected_id = n;
          selected_object = nullptr;
        }
      }
      ImGui::EndListBox();
    }

    if (model.selected_id >= 0 && model.selected_id < num_objects)
    {
      ImGui::Separator();
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SELECTED OBJECT");
      ImGui::Separator();

      hhittable_base* selected_obj = get_editor_app()->scene_root->objects[model.selected_id];
      selected_object = selected_obj;

      selected_obj->accept(vdraw_edit_panel());
    }
  }

  void feditor_window::draw_new_object_panel(fnew_object_panel_model& model)
  {
    if (ImGui::Button("Add new"))
    {
      ImGui::OpenPopup("New object?");
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("New object?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
      model.c_model.objects = REG.get_classes();
      std::string hittable_class_name = hhittable_base::get_class_static()->get_class_name();

      fui_helper::draw_selection_combo<oclass_object>(model.c_model, "Class", [=](const oclass_object* obj) -> bool{ return obj->get_parent_class_name() == hittable_class_name; }, nullptr);

      if (ImGui::Button("Add", ImVec2(120, 0)) && model.c_model.selected_object != nullptr)
      {
        get_editor_app()->scene_root->add(REG.spawn_from_class<hhittable_base>(model.c_model.selected_object));
        ImGui::CloseCurrentPopup();
      }
      ImGui::SetItemDefaultFocus();
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0)))
      {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  void feditor_window::draw_delete_object_panel(fdelete_object_panel_model& model)
  {
    if (ImGui::Button("Delete selected"))
    {
      ImGui::OpenPopup("Delete object?");
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Delete object?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
      hhittable_base* selected_obj = get_editor_app()->scene_root->objects[model.selected_id];
      if (selected_obj != nullptr)
      {
        ImGui::BeginDisabled(true);
        selected_obj->accept(vdraw_edit_panel());
        ImGui::EndDisabled();

        if (ImGui::Button("Delete", ImVec2(120, 0)))
        {
          get_editor_app()->scene_root->remove(model.selected_id);
          ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
          ImGui::CloseCurrentPopup();
        }
      }
      ImGui::EndPopup();
    }
  }
  void feditor_window::update_default_spawn_position()
  {
    fcamera& camera = get_editor_app()->scene_root->camera;

    // Find center of the scene, new objects scan be spawned there
    fvec3 look_from = camera.location;
    fvec3 look_dir = fmath::to_vec3(camera.forward);
    float dist_to_focus = 50.0f;

    // Ray to the look at position to find non colliding spawn point
    fray center_of_scene_ray(look_from, look_dir);
    fhit_record center_of_scene_hit;
    // TODO FIX
    //if (state.scene_root->hit(center_of_scene_ray, 2.0f*dist_to_focus, center_of_scene_hit))
    //{
    //  state.center_of_scene = center_of_scene_hit.p;
    //  state.distance_to_center_of_scene = fmath::length(center_of_scene_hit.p - look_from);
    //}
    //else
    //{
    //  state.center_of_scene = look_from - look_dir * dist_to_focus;
    //  state.distance_to_center_of_scene = dist_to_focus;
    //}
  }
  
  void feditor_window::handle_input()
  {
    ImGuiIO& io = ImGui::GetIO();
    fapplication* app = fapplication::get_instance();
    fcamera& camera = app->scene_root->camera;

    // Handle screen selection
    if (!ImGui::IsMouseHoveringAnyWindow() && ImGui::IsMouseClicked(ImGuiMouseButton_Left, false))
    {
      if(!app->scene_root)
      {
        LOG_WARN("Can't trace scene, no scene!");
        return;
      }
      if(!app->physics_world)
      {
        LOG_WARN("Can't trace scene, no physics world!")
        return;
      }
      RECT rect;
      if(!GetWindowRect(get_window_handle(), &rect))
      {
        LOG_WARN("Can't trace scene, GetWindowRect failed!")
        return;
      }
      POINT point;
      if (!GetCursorPos(&point))
      {
        LOG_WARN("Can't trace scene, GetCursorPos failed!")
        return;
      }

      // World space ray based on screen space click coordinates
      const fray ray = camera.get_ray(width, height, static_cast<uint32_t>(point.x - rect.left), static_cast<uint32_t>(point.y - rect.top));
      constexpr float ray_length = 10000.0f;
      const fvec3 b = ray.at(ray_length);

      // Physics scene querry
      const reactphysics3d::Vector3 px_a(camera.location.x, camera.location.y, camera.location.z);
      const reactphysics3d::Vector3 px_b(b.x, b.y, b.z);
      const reactphysics3d::Ray px_ray(px_a, px_b);
      raycast_callback px_callback;
      app->physics_world->raycast(px_ray, &px_callback);

      // Find rigid body in world
      bool found = false;
      if(reactphysics3d::Body* hit_body = px_callback.get_closest_body())
      {
        std::vector<hstatic_mesh*> meshes = REG.get_all_by_type<hstatic_mesh>();
        for (hstatic_mesh* m : meshes)
        {
          if (m->rigid_body == hit_body)
          {
            selected_object = m;
            found = true;
            break;
          }
        }
      }
      if(!found)
      {
        selected_object = nullptr;
      }
    }

    // Handle keyboard timings
    if (!io.WantCaptureKeyboard)
    {
      io.KeyRepeatDelay = 0.0f;
      io.KeyRepeatRate = 1.0f / 60.0f;
    }
    else
    {
      io.KeyRepeatDelay = 0.25f;
      io.KeyRepeatRate = 0.05f;
    }

    // Handle hotkeys
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
    {
      app->is_running = false;
    }
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space), false))
    {
      get_editor_app()->wants_to_simulate_physics = !get_editor_app()->wants_to_simulate_physics;
    }

    // Handle camera movement
    if (!io.WantCaptureKeyboard)
    {
      camera.move_speed += ImGui::GetIO().MouseWheel;
      camera.input_up = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_E)) ? 1 : 0;
      camera.input_down = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Q)) ? 1 : 0;
      camera.input_forward = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_W)) ? 1 : 0;
      camera.input_backward = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)) ? 1 : 0;
      camera.input_left = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)) ? 1 : 0;
      camera.input_right = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)) ? 1 : 0;
    }
    else
    {
      camera.input_up = 0;
      camera.input_down = 0;
      camera.input_forward = 0;
      camera.input_backward = 0;
      camera.input_left = 0;
      camera.input_right = 0;
    }

    // Handle camera rotation
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
      ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
      camera.input_yaw = static_cast<int>(mouse_delta.x);
      camera.input_pitch = static_cast<int>(mouse_delta.y);
    }
    else
    {
      camera.input_yaw = 0;
      camera.input_pitch = 0;
    }

    // Object movement
    if (!io.WantCaptureKeyboard)
    {
      fvec3 object_movement_axis = fvec3(0.0f, 0.0f, 0.0f);
      float mouse_delta = 0.0f;
      if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Z)))
      {
        object_movement_axis = fvec3(1.0f, 0.0f, 0.0f);
        mouse_delta = ImGui::GetIO().MouseDelta.x;
        if (camera.forward.z < 0.0f)
        {
          mouse_delta = -mouse_delta;
        }
      }
      else if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_C)))
      {
        object_movement_axis = fvec3(0.0f, -1.0f, 0.0f);
        mouse_delta = ImGui::GetIO().MouseDelta.y;
      }
      else if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_X)))
      {
        object_movement_axis = fvec3(0.0f, 0.0f, 1.0f);
        mouse_delta = ImGui::GetIO().MouseDelta.x;
        if (camera.forward.x > 0.0f)
        {
          mouse_delta = -mouse_delta;
        }
      }
      if (!fmath::is_zero(object_movement_axis) && mouse_delta != 0.0f && selected_object != nullptr)
      {
        fvec3 selected_origin = selected_object->origin;
        selected_object->origin = selected_origin + object_movement_axis * mouse_delta;
      }
    }
  }

}