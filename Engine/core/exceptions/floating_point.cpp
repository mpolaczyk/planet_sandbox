#include "core/exceptions/floating_point.h"

namespace engine
{
  ffpe_enabled_scope::ffpe_enabled_scope(unsigned int enable_bits)
  {
#if USE_FPEXCEPT
    _controlfp_s(&old_values, _MCW_EM, _MCW_EM);
    enable_bits &= _MCW_EM;
    _clearfp();
    _controlfp_s(0, ~enable_bits, old_values);
#endif
  }

  ffpe_enabled_scope::~ffpe_enabled_scope()
  {
#if USE_FPEXCEPT
    _controlfp_s(0, old_values, _MCW_EM);
#endif
  }

  ffpe_disabled_scope::ffpe_disabled_scope()
  {
#if USE_FPEXCEPT
    _controlfp_s(&old_values, 0, 0);
    _controlfp_s(0, _MCW_EM, _MCW_EM);
#endif
  }

  ffpe_disabled_scope::~ffpe_disabled_scope()
  {
#if USE_FPEXCEPT
    _clearfp();
    _controlfp_s(0, old_values, _MCW_EM);
#endif
  }
}
