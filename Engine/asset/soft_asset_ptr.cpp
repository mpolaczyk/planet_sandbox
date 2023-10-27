
#include "asset/soft_asset_ptr.h"
#include "engine/log.h"

namespace engine
{
  template<typename T>
  void soft_asset_ptr<T>::set_name(const std::string& in_name)
  {
    if (in_name != name)
    {
      name = in_name;
      object = nullptr;
    }
  }

  template<typename T>
  std::string soft_asset_ptr<T>::get_name() const
  {
    return name;
  }

  template<typename T>
  bool soft_asset_ptr<T>::is_loaded() const
  {
    return object != nullptr;
  }

  template<typename T>
  const T* soft_asset_ptr<T>::get() const
  {
    if (!is_loaded())
    {
      //object = globals::get_asset_registry()->find_asset<T>(name); FIX
      if (object == nullptr)
      {
        object = T::load(name);
        if (object != nullptr)
        {
          //globals::get_asset_registry()->add<T>(object, name); FIX
        }
        else
        {
          LOG_ERROR("Unable to find asset: {0}", name);
        }
      }
    }
    return object;
  }
}