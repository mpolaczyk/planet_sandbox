#include "stdafx.h"

#include "imgui.h"
#include "math/materials.h"

std::string material::get_name() const
{
  std::ostringstream oss;
  oss << "/" << material_type_names[(int)type] << "/" << name;
 return oss.str();
}

void material::draw_edit_panel()
{
  ImGui::Text("Material: ");
  ImGui::SameLine();
  ImGui::Text(get_name().c_str());
  
  int type_int = (int)type;
  ImGui::DragInt("Type", &type_int, 1, 1, 2);
  type = (material_type)type_int;

  ImGui::ColorEdit3("Color", color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
  ImGui::ColorEdit3("Emitted color", emitted_color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
  
  ImGui::DragFloat("Smoothness", &smoothness, 0.01f, 0.0f, 1.0f);
  
  ImGui::DragFloat("Gloss probability", &gloss_probability, 0.01f, 0.0f, 1.0f);
  ImGui::ColorEdit3("Gloss color", gloss_color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);

  ImGui::DragFloat("Refraction probability", &refraction_probability, 0.01f, 0.0f, 1.0f);
  ImGui::DragFloat("Refraction index", &refraction_index, 0.01f);
}
