#pragma once

#include <float.h>

#include "core/core.h"

/* Based on
 https://randomascii.wordpress.com/2012/04/21/exceptional-floating-point/

 The floating-point exception flags are part of the processor state which means that they are per-thread settings. 
 Therefore, if you want exceptions enabled everywhere you need to do it in each thread, 
 typically in main/WinMain and in your thread start function, by dropping an FPExceptionEnabler object in the top of these functions.
 
 When calling out to D3D or any code that may use floating-point in a way that triggers these exceptions you need to drop in an FPExceptionDisabler object.
 
 Alternately, if most your code is not FP exception clean then it may make more sense to leave FP exceptions disabled most of the time 
 and then enable them in particular areas, such as particle systems.
*/

namespace engine
{
  class ENGINE_API vec4
  {
  public:
    explicit vec4() = default;

    float x, y, z, w;
  };

  class ENGINE_API fpe_enabled_scope
  {
  public:
    fpe_enabled_scope(unsigned int enable_bits = _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID);
    ~fpe_enabled_scope();

  private:
    unsigned int old_values;

    fpe_enabled_scope(const fpe_enabled_scope&);
    fpe_enabled_scope& operator=(const fpe_enabled_scope&);
  };

  class ENGINE_API fpe_disabled_scope
  {
  public:
    fpe_disabled_scope();
    ~fpe_disabled_scope();

  private:
    unsigned int old_values;

    fpe_disabled_scope(const fpe_disabled_scope&);
    fpe_disabled_scope& operator=(const fpe_disabled_scope&);
  };
}


