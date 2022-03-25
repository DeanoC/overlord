#include "core/core.h"
#include "memory/memory.h"
#include "tiny_image_format/tiny_image_format_base.h"
#include "tiny_image_format/tiny_image_format_query.h"
#include "image/image.h"
#include "image/create.h"

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

Image_ImageHeader * Image_CreateWithCLUT(uint32_t width_, uint32_t height_, TinyImageFormat format_, uint32_t clutSize_, Memory_Allocator* memoryAllocator_) {
	return Image_CreateArrayWithCLUT(width_, height_, 1, format_, clutSize_, memoryAllocator_);
}

Image_ImageHeader * Image_CreateWithCLUTNoClear(uint32_t width_, uint32_t height_, TinyImageFormat format_, uint32_t clutSize_, Memory_Allocator* memoryAllocator_) {
	return Image_CreateArrayWithCLUTNoClear(width_, height_, 1, format_, clutSize_, memoryAllocator_);
}

Image_ImageHeader * Image_CreateArrayWithCLUT(uint32_t width_,
	                                         uint32_t height_,
	                                         uint32_t slices_,
	                                         TinyImageFormat format_,
	                                         uint32_t clutSize_,
	                                         Memory_Allocator* memoryAllocator_) {
	Image_ImageHeader* image = Image_CreateArrayWithCLUTNoClear(width_, height_, slices_, format_, clutSize_, memoryAllocator_);
	memset(Image_RawDataPtr(image), 0, image->dataSizeInBytes - sizeof(Image_ImageHeader));
	Image_ImageHeader *clut = Image_GetNextImageData(image);
	memset(Image_RawDataPtr(clut), 0, clut->dataSizeInBytes - sizeof(Image_ImageHeader));
	return image;
}

Image_ImageHeader * Image_CreateArrayWithCLUTNoClear(uint32_t width_,
                                                uint32_t height_,
                                                uint32_t slices_,
                                                TinyImageFormat format_,
                                                uint32_t clutSize_,
                                                Memory_Allocator* memoryAllocator_) {
	if(!TinyImageFormat_IsCLUT(format_)) return nullptr;

	// this creates the colour image and the lut image
	// currently LUT is always R8G8B8A8 format
	if(slices_ == 0) slices_ = 1;

	// if small enough create temporiries on the stack
	static const int TMP_STACK_SIZE = 32 * 1024;
	MEMORY_STACK_ALLOCATOR(_, TMP_STACK_SIZE);
	Memory_Allocator* tmpAllocator = memoryAllocator_;
	if( (Image_CalcSize(width_,height_, slices_, 1, format_) +
	     (Image_CalcSize(clutSize_, 1, 1, 1, TinyImageFormat_R8G8B8A8_UNORM))) < TMP_STACK_SIZE) {
		tmpAllocator = _;
	}

	Image_ImageHeader* image = (Image_ImageHeader*) Image_CreateNoClear(width_, height_, 1, slices_, format_, tmpAllocator);
	if(image) {
		Image_ImageHeader* clutImage = (Image_ImageHeader*) Image_CreateNoClear(clutSize_, 1, 1, 1, TinyImageFormat_R8G8B8A8_UNORM, tmpAllocator);
		if(clutImage) {
			image = Image_DestructiveJoinImages(image, clutImage, memoryAllocator_);
		} else {
			Image_Destroy(image);
			return nullptr;
		}
	}
	return image;
}