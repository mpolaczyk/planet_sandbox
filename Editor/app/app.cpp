#include "stdafx.h"

#include "imgui.h"

#include "app.h"
#include "hittables/hittables.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"

#include "renderer/renderer_base.h"

namespace editor
{
  fapp_instance::fapp_instance()
  {
    scene_root = hscene::spawn();
  }

  fapp_instance::~fapp_instance()
  {
    if (scene_root)
    {
      scene_root->destroy();
    }
  }

  void update_default_spawn_position(fapp_instance& state)
  {
    fcamera& camera = state.scene_root->camera_config;

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

  void handle_input(fapp_instance& state)
  {
    // Object selection
    // Hover the mouse over the input window and click to select.
    // User will hold LMB until the next frame is rendered. Renderer will output the object id texture.
    // Color under cursor will determine which hittable is hitted on the scene.
    // I do it this way because imported mesh data is bonkers. Mesh instances are merged together across the whole scene.
    // All of them have origin 0,0,0.
    // Line-box hit detection will not work properly. Line-mesh - no time for that.
    if (state.output_window_is_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
      state.scene_root->renderer->show_object_id = 1;

      std::vector<hstatic_mesh*> meshes = REG.get_all_by_type<hstatic_mesh>();
      bool found = false;
      for (hstatic_mesh* m : meshes)
      {
        XMUINT4 hash = fmath::uint32_to_colori(m->get_hash());

        if (state.output_window_cursor_color[0] == hash.x
          && state.output_window_cursor_color[1] == hash.y
          && state.output_window_cursor_color[2] == hash.z)
        {
          state.selected_object = m;
          found = true;
          break;
        }
      }
      if (!found)
      {
        state.selected_object = nullptr;
      }
    }
    else
    {
      state.scene_root->renderer->show_object_id = 0;
    }

    ImGuiIO& io = ImGui::GetIO();
    fcamera& camera = state.scene_root->camera_config;

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
      state.is_running = false;
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
      if (!fmath::is_zero(object_movement_axis) && mouse_delta != 0.0f && state.selected_object != nullptr)
      {
        fvec3 selected_origin = state.selected_object->origin;
        state.selected_object->origin = selected_origin + object_movement_axis * mouse_delta;
      }
    }
  }
}