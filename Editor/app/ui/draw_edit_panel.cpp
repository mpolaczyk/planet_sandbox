#include "stdafx.h"

#include "imgui.h"

#include "draw_edit_panel.h"

#include "hittables/hittables.h"
#include "hittables/light.h"
#include "hittables/scene.h"
#include "hittables/sphere.h"
#include "hittables/static_mesh.h"

namespace editor
{
  void vdraw_edit_panel::visit(hhittable_base& object) const
  {
    std::string name = object.get_display_name();
    assert(name.size() <= 256);
    char* buffer = new char[256];
    strcpy(buffer, name.c_str());
    if(ImGui::InputText("Name:", buffer, 256))
    {
      object.set_display_name(buffer);
    }
    delete[] buffer;
    
    ImGui::DragFloat3("Origin", object.origin.e);
    ImGui::DragFloat3("Rotation", object.rotation.e);
    ImGui::DragFloat3("Scale", object.scale.e);

    // material pointer is missing, go to: draw_objects_panel()
  }

  void vdraw_edit_panel::visit(hscene& object) const
  {
    object.hhittable_base::accept(vdraw_edit_panel());
  }

  void vdraw_edit_panel::visit(hstatic_mesh& object) const 
  {
    object.hhittable_base::accept(vdraw_edit_panel());
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

  void vdraw_edit_panel::visit(hsphere& object) const
  {
    object.hhittable_base::accept(vdraw_edit_panel());
    ImGui::InputFloat("Radius", &object.radius);
  }

  void vdraw_edit_panel::visit(hlight& object) const
  {
    object.hhittable_base::accept(vdraw_edit_panel());

    bool enabled = static_cast<bool>(object.properties.enabled);
    ImGui::Checkbox("Enabled", &enabled);
    object.properties.enabled = enabled;
    {
      DirectX::XMFLOAT4& temp = object.properties.color;
      float temp_arr[4] = { temp.x, temp.y, temp.z, temp.w };
      ImGui::ColorEdit4("Color", temp_arr, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
      temp = { temp_arr[0], temp_arr[1], temp_arr[2], temp_arr[3] };
    }
    ImGui::SliderInt("Light type", &object.properties.light_type, 0, 2);
    ImGui::Text("0 - Point");
    ImGui::Text("1 - Directional");
    {
      DirectX::XMFLOAT4& temp = object.properties.direction;
      float temp_arr[3] = { temp.x, temp.y, temp.z };
      ImGui::InputFloat3("Direction", temp_arr);
      temp = { temp_arr[0], temp_arr[1], temp_arr[2], 1.0f };
    }
    ImGui::Text("2 - Spot");
    ImGui::DragFloat("Angle", &object.properties.spot_angle, 0.01f, 0.0f, 3.15f);
    ImGui::Text("Attenuation:");
    ImGui::DragFloat("Constant", &object.properties.constant_attenuation, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Linear", &object.properties.linear_attenuation, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Quadratic", &object.properties.quadratic_attenuation, 0.01f, 0.0f, 1.0f);
  }

  void vdraw_edit_panel::visit(amaterial& object) const
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