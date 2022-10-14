#pragma once
#include "core/core.h"
#include "image/image.h"

#ifdef __cplusplus
extern "C"
{
#endif

void Image_CopyImageChain(Image_ImageHeader const *src, Image_ImageHeader *dst);
void Image_CopyImage(Image_ImageHeader const *src, Image_ImageHeader *dst);
void Image_CopySlice(Image_ImageHeader const *src, uint32_t sw,
										 Image_ImageHeader *dst, uint32_t dw);
void Image_CopyPage(Image_ImageHeader const *src, uint32_t sz, uint32_t sw,
										Image_ImageHeader *dst, uint32_t dz, uint32_t dw);
void Image_CopyRow(Image_ImageHeader const *src, uint32_t sy, uint32_t sz, uint32_t sw,
                   Image_ImageHeader *dst, uint32_t dy, uint32_t dz, uint32_t dw);
void Image_CopyPixel(Image_ImageHeader const *src, uint32_t sx, uint32_t sy, uint32_t sz, uint32_t sw,
                     Image_ImageHeader *dst, uint32_t dx, uint32_t dy, uint32_t dz, uint32_t dw);

Image_ImageHeader * Image_Clone(Image_ImageHeader const * image);
Image_ImageHeader * Image_CloneStructure(Image_ImageHeader const * image);

Image_ImageHeader * Image_PreciseConvert(Image_ImageHeader const * src, TinyImageFormat const newFormat);

#ifdef __cplusplus
}
#endif
