#pragma once

namespace DirectX
{
  struct XMFLOAT4;
}

namespace editor
{
  class fapp_instance;

  template<typename T>
  struct fselection_combo_model
  {
    // In
    std::vector<const T*> objects;
    // Out, selected by the widget
    int selected_id = 0;
    const T* selected_object = nullptr;

    void reset()
    {
      selected_object = nullptr;
      selected_id = -1;
    }
  };

  struct fui_helper
  {
    static void input_float3(const char* caption, DirectX::XMFLOAT4& value);
    static void color_edit4(const char* caption, DirectX::XMFLOAT4& value);
    static void check_box(const char* caption, int& value);
    static bool input_text(const char* caption, std::string& text);

    template<typename T>
    static void draw_selection_combo(fselection_combo_model<T>& model, const char* name, std::function<bool(const T*)> predicate, const T* default_selected_object);
  };
}