#include "core/rtti/object_registry.h"

#include "engine/asset/soft_asset_ptr.h"
#include "engine/asset/asset.h"
#include "engine/log.h"

namespace engine
{
  template <typename T>
  fsoft_asset_ptr<T>::fsoft_asset_ptr(const std::string& in_name)
  {
    set_name(in_name);
  }

  template <typename T>
  void fsoft_asset_ptr<T>::set_name(const std::string& in_name)
  {
    if(in_name != name)
    {
      name = in_name;
      asset_ptr = nullptr;
    }
  }

  template <typename T>
  std::string fsoft_asset_ptr<T>::get_name() const
  {
    return name;
  }

  template <typename T>
  bool fsoft_asset_ptr<T>::is_loaded() const
  {
    return asset_ptr != nullptr;
  }

  template <typename T>
  bool fsoft_asset_ptr<T>::is_set() const
  {
    return !name.empty();
  }
  
  template <typename T>
  const T* fsoft_asset_ptr<T>::get() const
  {
    if(!is_loaded() && !name.empty())
    {
      T* asset_ptr_temp = REG.find<T>([=](const T* obj) -> bool { return obj->name == name; });

      if(asset_ptr_temp == nullptr)
      {
        asset_ptr_temp = T::spawn();

        if(!asset_ptr_temp->load(name))
        {
          asset_ptr_temp->destroy();
          LOG_ERROR("Unable to find asset: {0}", name);
          return nullptr;
        }
      }
      asset_ptr = asset_ptr_temp;
    }
    return asset_ptr;
  }
  
  template <typename T>
  T* fsoft_asset_ptr<T>::get()
  {
    return const_cast<T*>(const_cast<const fsoft_asset_ptr<T>*>(this)->get());
  }
}
