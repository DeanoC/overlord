#include "core/core.h"
#include "memory/memory.h"
#include "tiny_image_format/tiny_image_format_base.h"
#include "tiny_image_format/tiny_image_format_query.h"
#include "gfx_image/image.h"
#include "gfx_image/create.h"

Image_ImageHeader * Image_Create1D(uint32_t width, enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	return Image_Create(width, 1, 1, 1, format, memoryAllocator);
}
Image_ImageHeader * Image_Create1DNoClear(uint32_t width, enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	return Image_CreateNoClear(width, 1, 1, 1, format, memoryAllocator);
}
Image_ImageHeader * Image_Create1DArray(uint32_t width, uint32_t slices, enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	return Image_Create(width, 1, 1, slices, format, memoryAllocator);
}
Image_ImageHeader * Image_Create1DArrayNoClear(uint32_t width, uint32_t slices, enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	return Image_CreateNoClear(width, 1, 1, slices, format, memoryAllocator);
}

Image_ImageHeader * Image_Create2D(uint32_t width, uint32_t height, enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	return Image_Create(width, height, 1, 1, format, memoryAllocator);
}
Image_ImageHeader * Image_Create2DNoClear(uint32_t width, uint32_t height, enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	return Image_CreateNoClear(width, height, 1, 1, format, memoryAllocator);
}
Image_ImageHeader * Image_Create2DArray(uint32_t width,
                                                       uint32_t height,
                                                       uint32_t slices,
                                                       enum TinyImageFormat format,
																											 Memory_Allocator* memoryAllocator) {
	return Image_Create(width, height, 1, slices, format, memoryAllocator);
}
Image_ImageHeader * Image_Create2DArrayNoClear(uint32_t width,
                                                              uint32_t height,
                                                              uint32_t slices,
                                                              enum TinyImageFormat format,
                                                              Memory_Allocator* memoryAllocator) {
	return Image_CreateNoClear(width, height, 1, slices, format, memoryAllocator);
}

Image_ImageHeader * Image_Create3D(uint32_t width, uint32_t height, uint32_t depth, enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	return Image_Create(width, height, depth, 1, format, memoryAllocator);
}
Image_ImageHeader * Image_Create3DNoClear(uint32_t width,
                                                         uint32_t height,
                                                         uint32_t depth,
                                                         enum TinyImageFormat format,
                                                         Memory_Allocator* memoryAllocator) {
	return Image_CreateNoClear(width, height, depth, 1, format, memoryAllocator);
}
Image_ImageHeader * Image_Create3DArray(uint32_t width,
                                                       uint32_t height,
                                                       uint32_t depth,
                                                       uint32_t slices,
                                                       enum TinyImageFormat format,
                                                       Memory_Allocator* memoryAllocator) {
	return Image_Create(width, height, depth, slices, format, memoryAllocator);
}
Image_ImageHeader * Image_Create3DArrayNoClear(uint32_t width,
                                                              uint32_t height,
                                                              uint32_t depth,
                                                              uint32_t slices,
                                                              enum TinyImageFormat format,
                                                              Memory_Allocator* memoryAllocator) {
	return Image_CreateNoClear(width, height, depth, slices, format, memoryAllocator);
}

Image_ImageHeader * Image_CreateCubemap(uint32_t width, uint32_t height, enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	Image_ImageHeader * image = (Image_ImageHeader *)Image_Create(width, height, 1, 6, format, memoryAllocator);
	if(image) {
		image->flags = Image_Flag_Cubemap;
	}
	return image;
}
Image_ImageHeader * Image_CreateCubemapNoClear(uint32_t width, uint32_t height, enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	Image_ImageHeader * image = (Image_ImageHeader *)Image_CreateNoClear(width, height, 1, 6, format, memoryAllocator);
	if(image) {
		image->flags = Image_Flag_Cubemap;
	}
	return image;
}
Image_ImageHeader * Image_CreateCubemapArray(uint32_t width,
                                                            uint32_t height,
                                                            uint32_t slices,
                                                            enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	if(slices == 0) slices = 1;
	Image_ImageHeader * image = (Image_ImageHeader*) Image_Create(width, height, 1, slices * 6, format, memoryAllocator);
	if(image) {
		image->flags = Image_Flag_Cubemap;
	}
	return image;
}
Image_ImageHeader * Image_CreateCubemapArrayNoClear(uint32_t width,
                                                                 uint32_t height,
                                                                 uint32_t slices,
                                                                 enum TinyImageFormat format, Memory_Allocator* memoryAllocator) {
	if(slices == 0) slices = 1;
	Image_ImageHeader* image = (Image_ImageHeader*) Image_CreateNoClear(width, height, 1, slices * 6, format, memoryAllocator);
	if(image) {
		image->flags = Image_Flag_Cubemap;
	}
	return image;
}

Image_ImageHeader * Image_CreateCLUT(uint32_t width, uint32_t height, TinyImageFormat format, uint32_t clutSize, Memory_Allocator* memoryAllocator) {
	return Image_CreateCLUTArray(width, height, 1, format, clutSize, memoryAllocator);
}

Image_ImageHeader * Image_CreateCLUTNoClear(uint32_t width, uint32_t height, TinyImageFormat format, uint32_t clutSize, Memory_Allocator* memoryAllocator) {
	return Image_CreateCLUTArrayNoClear(width, height, 1, format, clutSize, memoryAllocator);
}

Image_ImageHeader * Image_CreateCLUTArray(uint32_t width,
                                                         uint32_t height,
                                                         uint32_t slices,
                                                         TinyImageFormat format,
                                                         uint32_t clutSize,
                                                         Memory_Allocator* memoryAllocator) {
	if(!TinyImageFormat_IsCLUT(format)) return nullptr;

	// this creates the colour image and the lut image
	// currently LUT is always R8G8B8A8 format
	if(slices == 0) slices = 1;
	Image_ImageHeader* image = (Image_ImageHeader*) Image_Create(width, height, 1, slices, format, memoryAllocator);
	if(image) {
		Image_ImageHeader* clutImage = (Image_ImageHeader*) Image_Create(clutSize, 1, 1, 1, TinyImageFormat_R8G8B8A8_UNORM, memoryAllocator);
		if(clutImage) {
			image->flags = Image_Flag_CLUT;
			image->nextType = Image_NT_CLUT;
			image->nextImage  = clutImage;
		} else {
			Image_Destroy(image);
			return nullptr;
		}
	}
	return image;
}
Image_ImageHeader * Image_CreateCLUTArrayNoClear(uint32_t width,
                                                                uint32_t height,
                                                                uint32_t slices,
                                                                TinyImageFormat format,
                                                                uint32_t clutSize,
                                                                Memory_Allocator* memoryAllocator) {
	if(!TinyImageFormat_IsCLUT(format)) return nullptr;

	// this creates the colour image and the lut image
	// currently LUT is always R8G8B8A8 format
	if(slices == 0) slices = 1;
	Image_ImageHeader* image = (Image_ImageHeader*) Image_CreateNoClear(width, height, 1, slices, format, memoryAllocator);
	if(image) {
		Image_ImageHeader* clutImage = (Image_ImageHeader*) Image_CreateNoClear(clutSize, 1, 1, 1, TinyImageFormat_R8G8B8A8_UNORM, memoryAllocator);
		if(clutImage) {
			image->flags = Image_Flag_CLUT;
			image->nextType = Image_NT_CLUT;
			image->nextImage  = clutImage;
		} else {
			Image_Destroy(image);
			return nullptr;
		}
	}

	return image;
}