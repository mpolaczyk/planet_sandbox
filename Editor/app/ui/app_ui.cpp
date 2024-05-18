#include "stdafx.h"

#include <stdio.h>
#include "core/windows_minimal.h"

#include "draw_edit_panel.h"
#include "imgui.h"

#include "app/app.h"
#include "math/chunk_generator.h"
#include "renderer/renderer_base.h"
#include "math/camera.h"

#include "core/core.h"
#include "engine.h"
#include "engine/string_tools.h"
#include "hittables/hittables.h"
#include "hittables/static_mesh.h"
#include "hittables/scene.h"
#include "resources/ffbx.h"

namespace editor
{
  void draw_editor_window(feditor_window_model& model, fapp_instance& state)
  {
    ImGui::Begin("EDITOR", nullptr);
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "STATS");
    ImGui::Separator();
    ImGui::Text("Application %.3f ms/frame", state.app_delta_time_ms);
    ImGui::Text("Renderer %.3f ms/frame", state.render_delta_time_ms);
    draw_hotkeys_panel(state);
    draw_materials_panel(model.rp_model, state);
    draw_object_registry_panel();
    ImGui::End();
  }
  void draw_hotkeys_panel(fapp_instance& state)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "HOTKEYS");
    ImGui::Separator();
    ImGui::Text("LMB (on image) - select object");
    ImGui::Text("Scroll - Camera speed (current speed: %f)", state.scene_root->camera_config.move_speed);
    ImGui::Text("QWEASD - Camera movement");
    ImGui::Text("RMB - Camera rotation");
    ImGui::Text("ZXC + mouse - Object movement");
  }
  void draw_materials_panel(fmaterials_panel_model& model, fapp_instance& state)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "MATERIALS");
    ImGui::Separator();
    if (ImGui::MenuItem("SAVE ALL"))
    {
      state.save_materials();
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
  void draw_object_registry_panel()
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

  void draw_scene_window(fscene_window_model& model, fapp_instance& state)
  {
    ImGui::Begin("SCENE", nullptr);
    if (ImGui::MenuItem("SAVE SCENE"))
    {
      state.save_scene_state();
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SAVED!");
    }
    ImGui::Separator();

    if (ImGui::MenuItem("LOAD FROM FBX"))
    {
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Importing!");
      engine::ffbx::load_fbx_assimp(model.import_file, state.scene_root);
    }
    fui_helper::input_text("FBX file", model.import_file);

    draw_renderer_panel(model.rp_model, state);
    draw_camera_panel(state);
    draw_scene_panel(state);
    draw_scene_objects_panel(model.op_model, state);
    ImGui::End();
  }
  void draw_renderer_panel(frenderer_panel_model& model, fapp_instance& state)
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
    rrenderer_base* renderer = state.scene_root->renderer;
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
      state.scene_root->renderer = renderer;
    }
    ImGui::Separator();

    // Renderer edit panel
    state.scene_root->renderer->accept(vdraw_edit_panel());
    ImGui::Separator();

    if (state.scene_root->renderer == nullptr)
    {
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "No renderer active");
    }
  }
  void draw_camera_panel(fapp_instance& state)
  {
    fcamera& camera = state.scene_root->camera_config;
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
  void draw_scene_panel(fapp_instance& state)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SCENE");
    ImGui::Separator();

    DirectX::XMFLOAT4& temp = state.scene_root->ambient_light_color;
    float temp_arr[4] = { temp.x,temp.y,temp.z,temp.w };
    ImGui::ColorEdit4("Ambient", temp_arr, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
    temp = { temp_arr[0],temp_arr[1],temp_arr[2],temp_arr[3] };

    DirectX::XMVECTORF32& temp1 = state.scene_root->clear_color;
    float temp_arr1[4] = { temp1.f[0],temp1.f[1],temp1.f[2],temp1.f[3] };
    ImGui::ColorEdit4("Clear", temp_arr1, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
    temp1 = { temp_arr1[0],temp_arr1[1],temp_arr1[2],temp_arr1[3] };
  }
  void draw_scene_objects_panel(fobjects_panel_model& model, fapp_instance& state)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SCENE OBJECTS");
    ImGui::Separator();

    draw_new_object_panel(model.nop_model, state);
    ImGui::SameLine();
    draw_delete_object_panel(model.d_model, state);

    fui_helper::input_text("Name filter", model.object_name_filter);

    int num_objects = (int)state.scene_root->objects.size();
    if (ImGui::BeginListBox("Objects", ImVec2(-FLT_MIN, fmath::min1(20, (float)num_objects + 1) * ImGui::GetTextLineHeightWithSpacing())))
    {
      for (int n = 0; n < num_objects; n++)
      {
        hhittable_base* obj = state.scene_root->objects[n];
        if (state.selected_object != nullptr && obj == state.selected_object)
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
          state.selected_object = nullptr;
        }
      }
      ImGui::EndListBox();
    }

    if (model.selected_id >= 0 && model.selected_id < num_objects)
    {
      ImGui::Separator();
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SELECTED OBJECT");
      ImGui::Separator();

      hhittable_base* selected_obj = state.scene_root->objects[model.selected_id];
      state.selected_object = selected_obj;

      selected_obj->accept(vdraw_edit_panel());
    }
  }

  void get_color_under_cursor(uint8_t& out_r, uint8_t& out_g, uint8_t& out_b)
  {
    out_r = 0;
    out_g = 0;
    out_b = 0;
    if (HDC hDC = GetDC(nullptr))
    {
      POINT point;
      if (GetCursorPos(&point))
      {
        COLORREF color = GetPixel(hDC, point.x, point.y);
        if (color != CLR_INVALID)
        {
          ReleaseDC(GetDesktopWindow(), hDC);
          out_r = static_cast<uint8_t>(GetRValue(color));
          out_g = static_cast<uint8_t>(GetGValue(color));
          out_b = static_cast<uint8_t>(GetBValue(color));
        }
      }
    }
  }

  void draw_output_window(foutput_window_model& model, fapp_instance& state)
  {
    if (state.scene_root->renderer->output_texture)
    {
      ImGui::Begin("OUTPUT", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::InputFloat("Zoom", &model.zoom, 0.1f);
      const rrenderer_base* renderer = state.scene_root->renderer;
      ImVec2 size = ImVec2(renderer->output_width * model.zoom, renderer->output_height * model.zoom);
      ImGui::Image((ImTextureID)renderer->output_srv.Get(), size, ImVec2(0, 0), ImVec2(1, 1));

      state.output_window_is_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
      state.output_window_is_hovered = ImGui::IsItemHovered();
      get_color_under_cursor(state.output_window_cursor_color[0], state.output_window_cursor_color[1], state.output_window_cursor_color[2]);

      ImGui::End();
    }
  }

  void draw_new_object_panel(fnew_object_panel_model& model, fapp_instance& state)
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
        state.scene_root->add(REG.spawn_from_class<hhittable_base>(model.c_model.selected_object));
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

  void draw_delete_object_panel(fdelete_object_panel_model& model, fapp_instance& state)
  {
    if (ImGui::Button("Delete selected"))
    {
      ImGui::OpenPopup("Delete object?");
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Delete object?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
      hhittable_base* selected_obj = state.scene_root->objects[model.selected_id];
      if (selected_obj != nullptr)
      {
        ImGui::BeginDisabled(true);
        selected_obj->accept(vdraw_edit_panel());
        ImGui::EndDisabled();

        if (ImGui::Button("Delete", ImVec2(120, 0)))
        {
          state.scene_root->remove(model.selected_id);
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


}