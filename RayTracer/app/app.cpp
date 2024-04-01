#include "stdafx.h"

#include "imgui.h"

#include "app.h"
#include "hittables/hittables.h"

#include "renderer/renderer_base.h"
#include "renderers/cpu_renderer_preview.h"
#include "renderers/cpu_renderer_reference.h"

namespace ray_tracer
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
    fvec3 look_from = state.camera_conf.look_from;
    fvec3 look_dir = state.camera_conf.look_dir;
    float dist_to_focus = state.camera_conf.dist_to_focus;

    // Ray to the look at position to find non colliding spawn point
    fray center_of_scene_ray(look_from, look_dir);
    fhit_record center_of_scene_hit;
    if (state.scene_root->hit(center_of_scene_ray, 2.0f*dist_to_focus, center_of_scene_hit))
    {
      state.center_of_scene = center_of_scene_hit.p;
      state.distance_to_center_of_scene = fmath::length(center_of_scene_hit.p - look_from);
    }
    else
    {
      state.center_of_scene = look_from - look_dir * dist_to_focus;
      state.distance_to_center_of_scene = dist_to_focus;
    }
  }

  void handle_input(fapp_instance& state)
  {
    // Handle clicks on the output window - select the object under the cursor
    if (state.output_window_lmb_x > 0.0f && state.output_window_lmb_y > 0.0f)
    {
      float u = state.output_window_lmb_x / (state.output_width - 1);
      float v = state.output_window_lmb_y / (state.output_height - 1);
      v = 1.0f - v; // because vertical axis is flipped in the output window
      fcamera cam;
      cam.configure(state.camera_conf);
      fray r = cam.get_ray(u, v);
      fhit_record hit;
      if (state.scene_root->hit(r, fmath::infinity, hit))
      {
        state.selected_object = hit.object;
      }

      state.output_window_lmb_x = -1.0f;
      state.output_window_lmb_y = -1.0f;
    }

    const ImGuiIO& io = ImGui::GetIO();

    // Handle hotkeys
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
    {
      state.is_running = false;
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F1)))
    {
      if (state.renderer_conf.type == rcpu_preview::get_class_static())
      {
        state.rw_model.rp_model.render_pressed = true;
      }
      else
      {
        state.renderer_conf.type = rcpu_preview::get_class_static();
      }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2)))
    {
      if (state.renderer_conf.type == rcpu_reference::get_class_static())
      {
        state.rw_model.rp_model.render_pressed = true;
      }
      else
      {
        state.renderer_conf.type = rcpu_reference::get_class_static();
      }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F5)))
    {
      state.rw_model.rp_model.render_pressed = true;
    }

    // Handle speed
    float wheel_delta = ImGui::GetIO().MouseWheel;
    state.move_speed = fmath::max1(10.5f, state.move_speed + wheel_delta / 2.0f);

    // Handle camera movement
    if (!io.WantCaptureKeyboard)
    {
      state.camera_conf.input_e = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_E)) ? 1 : 0;
      state.camera_conf.input_q = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Q)) ? 1 : 0;
      state.camera_conf.input_w = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_W)) ? 1 : 0;
      state.camera_conf.input_s = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)) ? 1 : 0;
      state.camera_conf.input_a = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)) ? 1 : 0;
      state.camera_conf.input_d = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)) ? 1 : 0;
    }
    state.camera_conf.move_speed = state.move_speed;
    
    // Handle camera rotation
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
      ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
      mouse_delta.x *= -1;
      mouse_delta.y *= -1;
      
      // Check if the mouse has moved
      if (mouse_delta.x != 0.0f || mouse_delta.y != 0.0f)
      {
        float delta_yaw = mouse_delta.x * state.rotate_speed;
        float delta_pitch = mouse_delta.y * state.rotate_speed;
        state.camera_conf.rotate(delta_yaw, delta_pitch);
      }
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
        if (state.camera_conf.look_dir.z < 0.0f)
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
        if (state.camera_conf.look_dir.x > 0.0f)
        {
          mouse_delta = -mouse_delta;
        }
      }
      if (!fmath::is_zero(object_movement_axis) && mouse_delta != 0.0f && state.selected_object != nullptr)
      {
        fvec3 selected_origin = state.selected_object->get_origin();
        state.selected_object->set_origin(selected_origin + object_movement_axis * mouse_delta);
      }
    }
  }
}