#pragma once

#include "core/core.h"
#include "gfx_image/image.h"

// helpers
EXTERN_C Image_ImageHeader const * Image_Create1D(uint32_t width, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create1DNoClear(uint32_t width, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create1DArray(uint32_t width, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create1DArrayNoClear(uint32_t width, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create2D(uint32_t width, uint32_t height, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create2DNoClear(uint32_t width, uint32_t height, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create2DArray(uint32_t width, uint32_t height, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create2DArrayNoClear(uint32_t width, uint32_t height, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create3D(uint32_t width, uint32_t height, uint32_t depth, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create3DNoClear(uint32_t width, uint32_t height, uint32_t depth, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create3DArray(uint32_t width, uint32_t height, uint32_t depth, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_Create3DArrayNoClear(uint32_t width, uint32_t height, uint32_t depth, uint32_t slices, TinyImageFormat format,struct Memory_Allocator* memoryAllocator);

EXTERN_C Image_ImageHeader const * Image_CreateCubemap(uint32_t width, uint32_t height, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_CreateCubemapNoClear(uint32_t width, uint32_t height, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_CreateCubemapArray(uint32_t width, uint32_t height, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_CreateCubemapArrayNoClear(uint32_t width, uint32_t height, uint32_t slices, TinyImageFormat format, struct Memory_Allocator* memoryAllocator);


EXTERN_C Image_ImageHeader const * Image_CreateCLUT(uint32_t width, uint32_t height, TinyImageFormat format, uint32_t clutSize, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_CreateCLUTNoClear(uint32_t width, uint32_t height, TinyImageFormat format, uint32_t clutSize, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_CreateCLUTArray(uint32_t width, uint32_t height, uint32_t slices, TinyImageFormat format, uint32_t clutSize, struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const * Image_CreateCLUTArrayNoClear(uint32_t width, uint32_t height, uint32_t slices, TinyImageFormat format, uint32_t clutSize, struct Memory_Allocator* memoryAllocator);