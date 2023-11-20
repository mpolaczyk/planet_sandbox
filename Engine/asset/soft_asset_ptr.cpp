
#include "asset/soft_asset_ptr.h"
#include "object/object_registry.h"
#include "engine/log.h"

namespace engine
{
  template<derives_from<asset_base> T>
  void soft_asset_ptr<T>::set_name(const std::string& in_name)
  {
    if (in_name != name)
    {
      name = in_name;
      asset_ptr = nullptr;
    }
  }

  template<derives_from<asset_base> T>
  std::string soft_asset_ptr<T>::get_name() const
  {
    return name;
  }

  template<derives_from<asset_base> T>
  bool soft_asset_ptr<T>::is_loaded() const
  {
    return asset_ptr != nullptr;
  }

  template<derives_from<asset_base> T>
  const T* soft_asset_ptr<T>::get() const
  {
    if (!is_loaded())
    {
      asset_ptr = get_object_registry()->find<T>(name);
      if (asset_ptr == nullptr)
      {
        asset_ptr = T::load(name);
        if (asset_ptr != nullptr)
        {
          get_object_registry()->add<T>(asset_ptr, name);
        }
        else
        {
          LOG_ERROR("Unable to find asset: {0}", name);
        }
      }
    }
    return asset_ptr;
  }
}