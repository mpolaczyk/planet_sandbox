#include "fpexcept.h"

namespace engine
{
  fpe_enabled_scope::fpe_enabled_scope(unsigned int enable_bits)
  {
#if USE_FPEXCEPT
    _controlfp_s(&old_values, _MCW_EM, _MCW_EM);
    enable_bits &= _MCW_EM;
    _clearfp();
    _controlfp_s(0, ~enable_bits, old_values);
#endif
  }

  fpe_enabled_scope::~fpe_enabled_scope()
  {
#if USE_FPEXCEPT
    _controlfp_s(0, old_values, _MCW_EM);
#endif
  }

  fpe_disabled_scope::fpe_disabled_scope()
  {
#if USE_FPEXCEPT
    _controlfp_s(&old_values, 0, 0);
    _controlfp_s(0, _MCW_EM, _MCW_EM);
#endif
  }

  fpe_disabled_scope::~fpe_disabled_scope()
  {
#if USE_FPEXCEPT
    _clearfp();
    _controlfp_s(0, old_values, _MCW_EM);
#endif
  }
}