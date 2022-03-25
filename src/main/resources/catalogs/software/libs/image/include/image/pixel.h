#pragma once

#include "core/core.h"
#include "core/math_real.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct Image_PixelF {
	float r;
	float g;
	float b;
	float a;
} Image_PixelF;

typedef struct Image_PixelD {
  double r;
  double g;
  double b;
  double a;
} Image_PixelD;

ALWAYS_INLINE void Image_PixelClampF(Image_PixelF *pixel, float const min[4], float const max[4]) {
  pixel->r = Math_Clamp_F(pixel->r, min[0], max[0]);
  pixel->g = Math_Clamp_F(pixel->g, min[1], max[1]);
  pixel->b = Math_Clamp_F(pixel->b, min[2], max[2]);
  pixel->a = Math_Clamp_F(pixel->a, min[3], max[3]);
}

ALWAYS_INLINE void Image_PixelClampD(Image_PixelD *pixel, double const min[4], double const max[4]) {
	pixel->r = Math_Clamp_D(pixel->r, min[0], max[0]);
	pixel->g = Math_Clamp_D(pixel->g, min[1], max[1]);
	pixel->b = Math_Clamp_D(pixel->b, min[2], max[2]);
	pixel->a = Math_Clamp_D(pixel->a, min[3], max[3]);
}

#ifdef __cplusplus
}
#endif
