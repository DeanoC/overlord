#pragma once
#include "core/core.h"
#include "dbg/assert.h"
#include "gfx_image/pixel.h"
#include "tiny_image_format/tiny_image_format_base.h"
#include "tiny_image_format/tiny_image_format_query.h"

// Images can have a chain of related images, this type declares what if any
// the next pointer are. Image_IT_None means no next images
// The user is responsible to setting the next type and alloc the next
// chains. Destroy will free the entire chain.
// MipMaps + Layers in the same image is not supported
typedef enum Image_NextType {
	Image_NT_None,
	Image_NT_MipMap,
	Image_NT_Layer,
	Image_NT_Plane,
	Image_NT_CLUT,
} Image_NextType;

typedef enum Image_FlagBits {
	Image_Flag_Cubemap = 0x1,				// slices are treated as faces of a cubemap
	Image_Flag_HeaderOnly = 0x2,		// no data attached
	Image_Flag_PackedMipMaps = 0x4,	// has a packed mipmap
	Image_Flag_CLUT = 0x8,					// has a Colour LookUp Table
} Image_FlagBits;

// Upto 4D (3D Arrays_ image data, stored as packed formats but
// accessed as floats or double upto 4 channels per pixel in RGBA order
// Support image arrays/slices
// You ask for R and it will retrieve it from wherever it really is in the
// format (i.e. you don't worry about how its encoded)
// Mipmaps can be stored either packed in the 1st image or in a chain of
// images. A utility function can convert from chain to packed.
// CLUTs index image has a LUT image next in chain. LUT should be R8G8B8A8

// the image data follows this header directly
typedef struct Image_ImageHeader {
	uint64_t dataSize; ///< size of data following this header (Not including it!)

	// width, height and depth are in pixels (NOT blocks)
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t slices;

	union {
		uint32_t fmtSizer; ///< ensure there is always 32 bit if saved to disk
		TinyImageFormat format; ///< type TinyImageFormat
	};

	uint16_t flags; ///< From Image_FlagBits
	uint8_t nextType; ///< Image_NextType
	uint8_t packedMipMapCount; // if has packed mip maps this it the level count

	union {
		uint64_t pad; // ensure always enough space for 64 bit if saved to disk
		struct Image_ImageHeader const *nextImage;
	};
	struct Memory_Allocator* memoryAllocator;

} Image_ImageHeader;

// Image are fundamentally 4D arrays
// 'helper' functions in create.h let you
// create and use them in more familiar texture terms
EXTERN_C Image_ImageHeader const *Image_Create(uint32_t width,
																							 uint32_t height,
																							 uint32_t depth,
																							 uint32_t slices,
																							 enum TinyImageFormat format,
																							 struct Memory_Allocator* memoryAllocator);
EXTERN_C Image_ImageHeader const *Image_CreateNoClear(uint32_t width,
																											uint32_t height,
																											uint32_t depth,
																											uint32_t slices,
																											enum TinyImageFormat format,
																											struct Memory_Allocator* memoryAllocator);
EXTERN_C void Image_Destroy(Image_ImageHeader const *image);

// if you want to use the calculation fields without an actual image
// this will fill in a valid header with no data or allocation
EXTERN_C void Image_FillHeader(uint32_t width,
															 uint32_t height,
															 uint32_t depth,
															 uint32_t slices,
															 enum TinyImageFormat format,
															 Image_ImageHeader *header);

EXTERN_C Image_ImageHeader const* Image_CreateHeaderOnly(uint32_t width,
																												 uint32_t height,
																												 uint32_t depth,
																												 uint32_t slices,
																												 enum TinyImageFormat format,
																												 struct Memory_Allocator* memoryAllocator);

EXTERN_C inline void *Image_RawDataPtr(Image_ImageHeader const *image) {
	assert(image != NULL);
	assert((image->flags & Image_Flag_HeaderOnly) == 0)
	return (void *) (image + 1);
}

EXTERN_C bool Image_GetBlocksAtF(Image_ImageHeader const *image, float *pixels, size_t blockCounts, size_t index);
EXTERN_C bool Image_SetBlocksAtF(Image_ImageHeader const *image, float const *pixels, size_t blockCount, size_t index);

EXTERN_C bool Image_GetPixelAtF(Image_ImageHeader const *image, float *pixel, size_t index);
EXTERN_C bool Image_SetPixelAtF(Image_ImageHeader const *image, float const *pixel, size_t index);
EXTERN_C bool Image_GetRowAtF(Image_ImageHeader const *image, float *pixels, size_t index);
EXTERN_C bool Image_SetRowAtF(Image_ImageHeader const *image, float const *pixels, size_t index);

EXTERN_C bool Image_GetBlocksAtD(Image_ImageHeader const *image, double *pixels, size_t blockCounts, size_t index);
EXTERN_C bool Image_SetBlocksAtD(Image_ImageHeader const *image, double const *pixels, size_t blockCount, size_t index);

EXTERN_C bool Image_GetPixelAtD(Image_ImageHeader const *image, double *pixel, size_t index);
EXTERN_C bool Image_SetPixelAtD(Image_ImageHeader const *image, double const *pixel, size_t index);
EXTERN_C bool Image_GetRowAtD(Image_ImageHeader const *image, double *pixels, size_t index);
EXTERN_C bool Image_SetRowAtD(Image_ImageHeader const *image, double const *pixels, size_t index);

EXTERN_C void Image_CopyImage(Image_ImageHeader const *src, Image_ImageHeader const *dst);
EXTERN_C void Image_CopySlice(Image_ImageHeader const *src,
																		uint32_t sw,
																		Image_ImageHeader const *dst,
																		uint32_t dw);
EXTERN_C void Image_CopyPage(Image_ImageHeader const *src,
																	 uint32_t sz, uint32_t sw,
																	 Image_ImageHeader const *dst,
																	 uint32_t dz, uint32_t dw);
EXTERN_C void Image_CopyRow(Image_ImageHeader const *src,
																	uint32_t sy, uint32_t sz, uint32_t sw,
																	Image_ImageHeader const *dst,
																	uint32_t dy, uint32_t dz, uint32_t dw);
EXTERN_C void Image_CopyPixel(Image_ImageHeader const *src,
																		uint32_t sx, uint32_t sy, uint32_t sz, uint32_t sw,
																		Image_ImageHeader const *dst,
																		uint32_t dx, uint32_t dy, uint32_t dz, uint32_t dw);


EXTERN_C inline size_t Image_PixelCountPerRowOf(Image_ImageHeader const *image) {
	return image->width;
}
EXTERN_C inline size_t Image_PixelCountPerPageOf(Image_ImageHeader const *image) {
	return image->width * image->height;
}
EXTERN_C inline size_t Image_PixelCountPerSliceOf(Image_ImageHeader const *image) {
	return image->width * image->height * image->depth;
}
EXTERN_C inline size_t Image_PixelCountOf(Image_ImageHeader const *image) {
	return image->width * image->height * image->depth * image->slices;
}
EXTERN_C size_t Image_PixelCountOfImageChainOf(Image_ImageHeader const *image);

EXTERN_C inline size_t Image_CalculateIndex(Image_ImageHeader const *image,
																									uint32_t x,
																									uint32_t y,
																									uint32_t z,
																									uint32_t slice) {
	assert(image);
	assert(x < image->width);
	assert(y < image->height);
	assert(z < image->depth);
	assert(slice < image->slices);
	size_t const size1D = Image_PixelCountPerRowOf(image);
	size_t const size2D = Image_PixelCountPerPageOf(image);
	size_t const size3D = Image_PixelCountPerSliceOf(image);
	size_t const index = (slice * size3D) + (z * size2D) + (y * size1D) + x;
	return index;
}
EXTERN_C inline size_t Image_GetBlockIndex(Image_ImageHeader const *image,
																								 uint32_t x,
																								 uint32_t y,
																								 uint32_t z,
																								 uint32_t slice) {
	assert(image);
	assert(x < image->width);
	assert(y < image->height);
	assert(z < image->depth);
	assert(slice < image->slices);
	size_t const blockH = TinyImageFormat_HeightOfBlock(image->format);
	size_t const blockW = TinyImageFormat_WidthOfBlock(image->format);

	size_t const size1D = image->width / blockW;
	size_t const size2D = (image->width / blockW) * (image->height / blockH);
	size_t const size3D = size2D * image->depth;
	size_t const index = 	(slice * size3D) +
												(z * size2D) +
												((y / blockH) * size1D) +
												(x / blockW);
	return index;
}

EXTERN_C inline size_t Image_ByteCountPerRowOf(Image_ImageHeader const *image) {
	assert(image);

	return (Image_PixelCountPerRowOf(image) * TinyImageFormat_BitSizeOfBlock(image->format)) /
			(TinyImageFormat_PixelCountOfBlock(image->format) * 8);
	}
EXTERN_C inline size_t Image_ByteCountPerPageOf(Image_ImageHeader const *image) {
	assert(image);
	return (Image_PixelCountPerPageOf(image) * TinyImageFormat_BitSizeOfBlock(image->format)) /
			(TinyImageFormat_PixelCountOfBlock(image->format) * 8);
}
EXTERN_C inline size_t Image_ByteCountPerSliceOf(Image_ImageHeader const *image) {
	assert(image);
	return (Image_PixelCountPerSliceOf(image) * TinyImageFormat_BitSizeOfBlock(image->format)) /
				(TinyImageFormat_PixelCountOfBlock(image->format) * 8);

}
EXTERN_C inline size_t Image_ByteCountOf(Image_ImageHeader const *image) {
	assert(image);

	return (Image_PixelCountOf(image) * TinyImageFormat_BitSizeOfBlock(image->format)) /
						(TinyImageFormat_PixelCountOfBlock(image->format) * 8);

}

EXTERN_C size_t Image_ByteCountOfImageChainOf(Image_ImageHeader const *image);

EXTERN_C size_t Image_BytesRequiredForMipMapsOf(Image_ImageHeader const *image);

EXTERN_C size_t Image_LinkedImageCountOf(Image_ImageHeader const *image);

EXTERN_C size_t Image_MipMapCountOf(Image_ImageHeader const *image);

EXTERN_C Image_ImageHeader const *Image_LinkedImageOf(Image_ImageHeader const *image, size_t const index);

EXTERN_C inline bool Image_Is1D(Image_ImageHeader const *image) {
	return image->height == 1 && image->depth == 1;
}
EXTERN_C inline bool Image_Is2D(Image_ImageHeader const *image) {
	return image->height != 1 && image->depth == 1;
}
EXTERN_C inline bool Image_Is3D(Image_ImageHeader const *image) {
	return image->depth != 1;
}
EXTERN_C inline bool Image_IsArray(Image_ImageHeader const *image) {
	return image->slices != 1;
}
EXTERN_C inline bool Image_IsCubemap(Image_ImageHeader const *image) {
	return image->flags & Image_Flag_Cubemap;
}

EXTERN_C inline bool Image_HasPackedMipMaps(Image_ImageHeader const *image) {
	return image->flags & Image_Flag_PackedMipMaps;
}