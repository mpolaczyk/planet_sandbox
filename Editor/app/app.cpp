#include "stdafx.h"

#include "imgui.h"

#include "app.h"
#include "hittables/hittables.h"
#include "hittables/scene.h"

#include "renderer/renderer_base.h"

namespace editor
{
  fapp_instance::fapp_instance()
  {
    scene_root = hscene::spawn();
  }

  fapp_instance::~fapp_instance()
  {
    if (renderer)
    {
      renderer->destroy();
    }
    if (scene_root)
    {
      scene_root->destroy();
    }
  }

  void update_default_spawn_position(fapp_instance& state)
  {
    // Find center of the scene, new objects scan be spawned there
    fvec3 look_from = state.camera.location;
    fvec3 look_dir = fmath::to_vec3(state.camera.forward);
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
    // Handle clicks on the output window - select the object under the cursor
    if (state.output_window_lmb_x > 0.0f && state.output_window_lmb_y > 0.0f)
    {
      float u = state.output_window_lmb_x / (state.output_width - 1);
      float v = state.output_window_lmb_y / (state.output_height - 1);
      v = 1.0f - v; // because vertical axis is flipped in the output window

      LOG_ERROR("Selecting not implemented");
      
      // TODO FIX
      // fray r = state.camera.get_ray(u, v);
      // fhit_record hit;
      // if (state.scene_root->hit(r, fmath::infinity, hit))
      // {
      //   state.selected_object = hit.object;
      // }

      state.output_window_lmb_x = -1.0f;
      state.output_window_lmb_y = -1.0f;
    }

    const ImGuiIO& io = ImGui::GetIO();

    // Handle hotkeys
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
    {
      state.is_running = false;
    }
    
    // Handle camera movement
    if (!io.WantCaptureKeyboard)
    {
      state.camera.move_speed += ImGui::GetIO().MouseWheel;
      state.camera.input_up = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_E)) ? 1 : 0;
      state.camera.input_down = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Q)) ? 1 : 0;
      state.camera.input_forward = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_W)) ? 1 : 0;
      state.camera.input_backward = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)) ? 1 : 0;
      state.camera.input_left = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)) ? 1 : 0;
      state.camera.input_right = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)) ? 1 : 0;
    }
    
    // Handle camera rotation
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
      ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
      state.camera.input_yaw = mouse_delta.x;
      state.camera.input_pitch = mouse_delta.y;
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
        if (state.camera.forward.z < 0.0f)
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
        if (state.camera.forward.x > 0.0f)
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