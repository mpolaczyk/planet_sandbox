﻿#include "stdafx.h"

#include "imgui.h"

#include "draw_edit_panel.h"

#include "hittables/hittables.h"
#include "hittables/light.h"
#include "hittables/scene.h"
#include "hittables/sphere.h"
#include "hittables/static_mesh.h"

namespace editor
{
  void fdraw_edit_panel::visit(class hhittable_base& object) const
  {
    std::string hittable_name = object.get_display_name();
    ImGui::Text("Object: ");
    ImGui::SameLine();
    ImGui::Text(hittable_name.c_str());
    ImGui::DragFloat3("Origin", object.origin.e);
    ImGui::DragFloat3("Rotation", object.rotation.e);
    ImGui::DragFloat3("Scale", object.scale.e);
  }

  void fdraw_edit_panel::visit(class hscene& object) const
  {
    object.hhittable_base::accept(fdraw_edit_panel());
  }

  void fdraw_edit_panel::visit(class hstatic_mesh& object) const 
  {
    object.hhittable_base::accept(fdraw_edit_panel());
    {
      std::string name = object.mesh_asset_ptr.get_name();
      assert(name.size() <= 256);
      char* buffer = new char[256];
      strcpy(buffer, name.c_str());
      if (ImGui::InputText("Object file", buffer, 256))
      {
        object.mesh_asset_ptr.set_name(buffer);
      }
      delete[] buffer;
    }
  }

  void fdraw_edit_panel::visit(class hsphere& object) const
  {
    object.hhittable_base::accept(fdraw_edit_panel());
    ImGui::InputFloat("Radius", &object.radius);
  }

  void fdraw_edit_panel::visit(class hlight& object) const
  {
    object.hhittable_base::accept(fdraw_edit_panel());
    
    DirectX::XMFLOAT4& temp = object.properties.color;
    float temp_arr[4] = { temp.x, temp.y, temp.z, temp.w };
    ImGui::ColorEdit4("Color", temp_arr, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
    temp = { temp_arr[0], temp_arr[1], temp_arr[2], temp_arr[3] };
    
  }

  void fdraw_edit_panel::visit(class amaterial& object) const
  {
    ImGui::ColorEdit3("Color", object.color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
    ImGui::ColorEdit3("Emitted color", object.emitted_color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);

    ImGui::DragFloat("Smoothness", &object.smoothness, 0.01f, 0.0f, 1.0f);

    ImGui::DragFloat("Gloss probability", &object.gloss_probability, 0.01f, 0.0f, 1.0f);
    ImGui::ColorEdit3("Gloss color", object.gloss_color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);

    ImGui::DragFloat("Refraction probability", &object.refraction_probability, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Refraction index", &object.refraction_index, 0.01f);
  }
}