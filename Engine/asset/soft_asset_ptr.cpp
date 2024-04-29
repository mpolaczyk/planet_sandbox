
#include "asset/soft_asset_ptr.h"
#include "asset/asset.h"
#include "object/object_registry.h"
#include "engine/log.h"
#include <cassert>

namespace engine
{
  template<derives_from<aasset_base> T>
  void fsoft_asset_ptr<T>::set_name(const std::string& in_name)
  {
    if (in_name != name)
    {
      name = in_name;
      asset_ptr = nullptr;
    }
  }

  template<derives_from<aasset_base> T>
  std::string fsoft_asset_ptr<T>::get_name() const
  {
    return name;
  }

  template<derives_from<aasset_base> T>
  bool fsoft_asset_ptr<T>::is_loaded() const
  {
    return asset_ptr != nullptr;
  }

  template<derives_from<aasset_base> T>
  const T* fsoft_asset_ptr<T>::get() const
  {
    if (!is_loaded() && !name.empty())
    {
      T* asset_ptr_temp = REG.find<T>([=](const T* obj) -> bool { return obj->file_name == name; });
      
      if (asset_ptr_temp == nullptr)
      {
        asset_ptr_temp = T::spawn();

        if (!T::load(asset_ptr_temp, name))
        {
          asset_ptr_temp->destroy();
          LOG_ERROR("Unable to find asset: {0}", name);
          return nullptr;
        }
      }
      assert(asset_ptr_temp);
      asset_ptr = asset_ptr_temp;
    }
    return asset_ptr;
  }
}