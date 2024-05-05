#include "stdafx.h"

#include "imgui.h"

#include "draw_edit_panel.h"

#include "hittables/hittables.h"
#include "hittables/light.h"
#include "hittables/sphere.h"
#include "hittables/static_mesh.h"

namespace editor
{
  void vdraw_edit_panel::visit_hhittable_base(hhittable_base& object) const
  {
    std::string display_name = object.get_display_name();
    if (fui_helper::input_text("Name", display_name))
    {
      object.set_display_name(display_name);
    }
    
    ImGui::DragFloat3("Origin", object.origin.e);
    ImGui::DragFloat3("Rotation", object.rotation.e);
    ImGui::DragFloat3("Scale", object.scale.e);
  }

  void vdraw_edit_panel::visit(hstatic_mesh& object) const 
  {
    visit_hhittable_base(object);
    {
      std::string mesh_asset = object.mesh_asset_ptr.get_name();
      if (fui_helper::input_text("Mesh asset", mesh_asset))
      {
        object.mesh_asset_ptr.set_name(mesh_asset);
      }
    }
  }

  void vdraw_edit_panel::visit(hsphere& object) const
  {
    visit_hhittable_base(object);
    ImGui::InputFloat("Radius", &object.radius);
  }

  void vdraw_edit_panel::visit(hlight& object) const
  {
    std::string display_name = object.get_display_name();
    if (fui_helper::input_text("Name", display_name))
    {
      object.set_display_name(display_name);
    }
    ImGui::DragFloat3("Origin", object.origin.e);
    fui_helper::check_box("Enabled", object.properties.enabled);
    fui_helper::color_edit4("Color", object.properties.color);
    ImGui::SliderInt("Light type", &object.properties.light_type, 0, 2);
    ImGui::Text("0 - Point");
    ImGui::Text("1 - Directional");
    fui_helper::input_float3("Direction", object.properties.direction);
    ImGui::Text("2 - Spot");
    ImGui::DragFloat("Angle", &object.properties.spot_angle, 0.01f, 0.0f, 3.15f);
    ImGui::Text("Attenuation:");
    ImGui::DragFloat("Constant", &object.properties.constant_attenuation, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Linear", &object.properties.linear_attenuation, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Quadratic", &object.properties.quadratic_attenuation, 0.01f, 0.0f, 1.0f);
  }

  void vdraw_edit_panel::visit_aasset_base(aasset_base& object) const
  {
    ImGui::Text("File name: %s", object.file_name.c_str());
  }

  void vdraw_edit_panel::visit(amaterial& object) const
  {
    visit_aasset_base(object);
    fui_helper::color_edit4("Emissive", object.properties.emissive);
    fui_helper::color_edit4("Ambient", object.properties.ambient);
    fui_helper::color_edit4("Diffuse", object.properties.diffuse);
    fui_helper::color_edit4("Specular", object.properties.specular);
    ImGui::DragFloat("Specular power", &object.properties.specular_power, 0.01f, 0.0f, 1000.0f);
    fui_helper::check_box("Use texture", object.properties.use_texture);
    {
      std::string texture_asset = object.texture_asset_ptr.get_name();
      if (fui_helper::input_text("Texture asset", texture_asset))
      {
        object.texture_asset_ptr.set_name(texture_asset);
      }
    }
  }
}