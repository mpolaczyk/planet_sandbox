#pragma once
#include <concepts>

namespace engine
{
  template<class D, class B>
  concept derives_from = std::is_base_of<B, D>::value;
}

