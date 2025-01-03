#include "stdafx.h"

#include "app/editor_app.h"
#include "app/editor_window.h"

namespace editor
{
  void feditor_app::load_window_state() const
  {
    LOG_INFO("Loading: window state");

    std::string work_file = fio::get_window_file_path();
    std::ifstream input_stream(work_file.c_str());
    if (input_stream.fail())
    {
      LOG_ERROR("Unable to open file: {0}", work_file);
      return;
    }
    nlohmann::json j;
    input_stream >> j;

    input_stream.close();
  }

  void feditor_app::save_window_state() const
  {
    LOG_INFO("Saving: window state");

    nlohmann::json j;
    std::ofstream o(fio::get_window_file_path().c_str(), std::ios_base::out | std::ios::binary);
    std::string str = j.dump(2);
    if (o.is_open())
    {
      o.write(str.data(), str.length());
    }
    o.close();
  }
}