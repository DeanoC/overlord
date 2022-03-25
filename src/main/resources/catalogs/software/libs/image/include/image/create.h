#pragma once

#include "core/core.h"
#include "image/image.h"
#ifdef __cplusplus
extern "C"
{
#endif


// helpers
Image_ImageHeader * Image_Create1D(uint32_t width, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create1DNoClear(uint32_t width, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create1DArray(uint32_t width, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create1DArrayNoClear(uint32_t width, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create2D(uint32_t width, uint32_t height, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create2DNoClear(uint32_t width, uint32_t height, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create2DArray(uint32_t width, uint32_t height, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create2DArrayNoClear(uint32_t width, uint32_t height, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create3D(uint32_t width, uint32_t height, uint32_t depth, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create3DNoClear(uint32_t width, uint32_t height, uint32_t depth, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create3DArray(uint32_t width, uint32_t height, uint32_t depth, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_Create3DArrayNoClear(uint32_t width, uint32_t height, uint32_t depth, uint32_t slices, TinyImageFormat format,struct Memory_Allocator* memoryAllocator);

Image_ImageHeader * Image_CreateCubemap(uint32_t width, uint32_t height, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_CreateCubemapNoClear(uint32_t width, uint32_t height, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_CreateCubemapArray(uint32_t width, uint32_t height, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
Image_ImageHeader * Image_CreateCubemapArrayNoClear(uint32_t width, uint32_t height, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);


Image_ImageHeader * Image_CreateWithCLUT(uint32_t width_, uint32_t height_, TinyImageFormat format_, uint32_t clutSize_, struct Memory_Allocator* memoryAllocator_);
Image_ImageHeader * Image_CreateWithCLUTNoClear(uint32_t width_, uint32_t height_, TinyImageFormat format_, uint32_t clutSize_, struct Memory_Allocator* memoryAllocator_);
Image_ImageHeader * Image_CreateArrayWithCLUT(uint32_t width_, uint32_t height_, uint32_t slices_, TinyImageFormat format_, uint32_t clutSize_, struct Memory_Allocator* memoryAllocator_);
Image_ImageHeader * Image_CreateArrayWithCLUTNoClear(uint32_t width_, uint32_t height_, uint32_t slices_, TinyImageFormat format_, uint32_t clutSize_, struct Memory_Allocator* memoryAllocator_);

#ifdef __cplusplus
}
#endif
