#pragma once

#include "core/core.h"

#include "math/random.h"
#include "math/onb.h"
#include "math/math.h"


namespace engine
{
  struct ENGINE_API fpdf
  {
    // Return a random direction weighted by the internal PDF distribution
    virtual fvec3 get_direction() const = 0;

    // Return the corresponding PDF distribution value in that direction
    virtual float get_value(const fvec3& direction) const = 0;
  };


  struct ENGINE_API fsphere_pdf : public fpdf
  {
    fsphere_pdf() {}

    virtual fvec3 get_direction() const override
    {
      return frandom_cache::get_vec3();
    }

    virtual float get_value(const fvec3& direction) const override
    {
      return 1.0f / (4.0f * fmath::pi);
    }
  };

  struct ENGINE_API fcosine_pdf : public fpdf
  {
    fcosine_pdf() {}

    explicit fcosine_pdf(const fvec3& w)
    {
      uvw.build_from_w(w);
    }

    virtual fvec3 get_direction() const override
    {
      return uvw.local(frandom_cache::get_cosine_direction());
    }

    virtual float get_value(const fvec3& direction) const override
    {
      float cosine_theta = fmath::dot(fmath::normalize(direction), uvw.w);
      return (cosine_theta <= 0) ? 0 : cosine_theta / fmath::pi;
    }

    fonb uvw;
  };

  //struct ENGINE_API fhittable_pdf : public pdf
  //{
  //public:
  //  hittable_pdf(const hittable* _objects, const vec3& _origin)
  //    : object(_objects), origin(_origin)
  //  {}

  //  virtual vec3 get_direction() const override
  //  {
  //    return object->get_pdf_direction(origin);
  //  }

  //  virtual float get_value(const vec3& direction) const override
  //  {
  //    return object->get_pdf_value(origin, direction);
  //  }

  //public:
  //  const hittable* object;
  //  vec3 origin;
  //};

  class ENGINE_API fmixture_pdf : public fpdf
  {
  public:
    fmixture_pdf(fpdf* p0, fpdf* p1, float ratio)
      : ratio(ratio)
    {
      p[0] = p0;
      p[1] = p1;
    }

    float get_value(const fvec3& direction) const override
    {
      return 0.5f * p[0]->get_value(direction)
        + 0.5f * p[1]->get_value(direction);
    }

    fvec3 get_direction() const override
    {
      if (frandom_cache::get_float_0_1() > ratio)
        return p[0]->get_direction();
      else
        return p[1]->get_direction();
    }

  public:
    fpdf* p[2];
    float ratio = 0.5f;
  };
}