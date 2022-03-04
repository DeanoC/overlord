#include "core/core.h"
#include "dbg/assert.h"
#include "core/math.h"
#include "memory/memory.h"
#include "tiny_image_format/tiny_image_format_base.h"
#include "tiny_image_format/tiny_image_format_query.h"
#include "tiny_image_format/tiny_image_format_decode.h"
#include "tiny_image_format/tiny_image_format_encode.h"
#include "gfx_image/image.h"

EXTERN_C size_t Image_ByteCountOfImageChainOf(Image_ImageHeader const *image) {

	size_t total = Image_ByteCountOf(image);

	if(Image_HasPackedMipMaps(image)) {
		return total;
	}

	if(image->nextType != Image_NT_None && image->nextImage != nullptr) {
		total += Image_ByteCountOfImageChainOf(image->nextImage);
	}

	return total;
}
EXTERN_C size_t Image_LinkedImageCountOf(Image_ImageHeader const *image) {
  size_t count = 1;

	while (image && image->nextImage != nullptr) {
    count++;
    image = image->nextImage;
  }

  return count;
}

EXTERN_C size_t Image_MipMapCountOf(Image_ImageHeader const *image) {
	size_t count = 1;

	if(Image_HasPackedMipMaps(image)) {
		return image->packedMipMapCount;
	}

	while (image && image->nextType == Image_NT_MipMap && image->nextImage != nullptr) {
		count++;
		image = image->nextImage;
	}

	return count;
}

EXTERN_C Image_ImageHeader const *Image_LinkedImageOf(Image_ImageHeader const *image, size_t const index) {
  size_t count = 0;

  if(index > 0) {
		assert(Image_HasPackedMipMaps(image) == false)
	}

  do {
    if (count == index) {
      return image;
    }
    count++;
    image = image->nextImage;
  }  while (image);

return nullptr;
}

EXTERN_C bool Image_GetBlocksAtF(Image_ImageHeader const *image, float *pixels, size_t blockCount, size_t index) {
	assert(image);
	assert(pixels);

	if(!TinyImageFormat_CanDecodeLogicalPixelsF(image->format)) return false;

	uint32_t const pixelCount = TinyImageFormat_PixelCountOfBlock(image->format);

	memset(pixels, 0, pixelCount * sizeof(float) * 4);

	uint8_t *pixelPtr = ((uint8_t *) Image_RawDataPtr(image)) +
			index * (TinyImageFormat_BitSizeOfBlock(image->format) / 8);


	TinyImageFormat_DecodeInput input = { pixelPtr };
	return TinyImageFormat_DecodeLogicalPixelsF(image->format, &input, (uint32_t)blockCount, pixels);
}

EXTERN_C bool Image_SetBlocksAtF(Image_ImageHeader const *image, float const *pixels, size_t blockCount, size_t index) {
	assert(image);
	assert(pixels);

	if(!TinyImageFormat_CanEncodeLogicalPixelsF(image->format)) return false;

	uint32_t const pixelCount = TinyImageFormat_PixelCountOfBlock(image->format);

	uint8_t *pixelPtr = ((uint8_t *) Image_RawDataPtr(image)) +
			index * (TinyImageFormat_BitSizeOfBlock(image->format) / 8);


	TinyImageFormat_EncodeOutput output = { pixelPtr };
	return TinyImageFormat_EncodeLogicalPixelsF(image->format, pixels, (uint32_t)blockCount, &output);
}


EXTERN_C bool Image_GetPixelAtF(Image_ImageHeader const *image, float *pixel, size_t index) {
	assert(image);
	if(TinyImageFormat_PixelCountOfBlock(image->format) != 1) return false;
	return Image_GetBlocksAtF(image, pixel, 1, index);
}

EXTERN_C bool Image_SetPixelAtF(Image_ImageHeader const *image, float const *pixel, size_t index) {
	assert(image);
	if(TinyImageFormat_PixelCountOfBlock(image->format) != 1) return false;
	return Image_SetBlocksAtF(image, pixel, 1, index);
}

EXTERN_C bool Image_GetRowAtF(Image_ImageHeader const *image, float *pixels, size_t index) {
	assert(image);
	uint32_t const blockWidth = TinyImageFormat_WidthOfBlock(image->format);

	return Image_GetBlocksAtF(image, pixels, image->width / blockWidth, index);
}

EXTERN_C bool Image_SetRowAtF(Image_ImageHeader const *image, float const *pixels, size_t index) {
	assert(image);

	uint32_t const blockWidth = TinyImageFormat_WidthOfBlock(image->format);
	return Image_SetBlocksAtF(image, pixels, image->width / blockWidth, index);
}


EXTERN_C bool Image_GetBlocksAtD(Image_ImageHeader const *image, double *pixels, size_t blockCount, size_t index) {
	assert(image);
	assert(pixels);

	if(!TinyImageFormat_CanDecodeLogicalPixelsD(image->format)) return false;

	uint32_t const pixelCount = TinyImageFormat_PixelCountOfBlock(image->format);

	memset(pixels, 0, pixelCount * sizeof(float) * 4);

	uint8_t *pixelPtr = ((uint8_t *) Image_RawDataPtr(image)) +
			index * (TinyImageFormat_BitSizeOfBlock(image->format) / 8);


	TinyImageFormat_DecodeInput input = { pixelPtr };
	return TinyImageFormat_DecodeLogicalPixelsD(image->format, &input, (uint32_t)blockCount, pixels);
}

EXTERN_C bool Image_SetBlocksAtD(Image_ImageHeader const *image, double const *pixels, size_t blockCount, size_t index) {
	assert(image);
	assert(pixels);

	if(!TinyImageFormat_CanEncodeLogicalPixelsD(image->format)) return false;

	uint32_t const pixelCount = TinyImageFormat_PixelCountOfBlock(image->format);

	uint8_t *pixelPtr = ((uint8_t *) Image_RawDataPtr(image)) +
			index * (TinyImageFormat_BitSizeOfBlock(image->format) / 8);


	TinyImageFormat_EncodeOutput output = { pixelPtr };
	return TinyImageFormat_EncodeLogicalPixelsD(image->format, pixels, (uint32_t)blockCount, &output);
}

EXTERN_C bool Image_GetPixelAtD(Image_ImageHeader const *image, double *pixel, size_t index) {
	assert(image);
	if(TinyImageFormat_PixelCountOfBlock(image->format) != 1) return false;
	return Image_GetBlocksAtD(image, pixel, 1, index);
}

EXTERN_C bool Image_SetPixelAtD(Image_ImageHeader const *image, double const *pixel, size_t index) {
	assert(image);
	if(TinyImageFormat_PixelCountOfBlock(image->format) != 1) return false;
	return Image_SetBlocksAtD(image, pixel, 1, index);
}

EXTERN_C bool Image_GetRowAtD(Image_ImageHeader const *image, double *pixels, size_t index) {
	assert(image);
	uint32_t const blockWidth = TinyImageFormat_WidthOfBlock(image->format);

	return Image_GetBlocksAtD(image, pixels, image->width / blockWidth, index);
}

EXTERN_C bool Image_SetRowAtD(Image_ImageHeader const *image, double const *pixels, size_t index) {
	assert(image);

	uint32_t const blockWidth = TinyImageFormat_WidthOfBlock(image->format);
	return Image_SetBlocksAtD(image, pixels, image->width / blockWidth, index);
}


EXTERN_C size_t Image_BytesRequiredForMipMapsOf(Image_ImageHeader const *image) {

	uint32_t const maxMipLevels =
			Math_LogTwo_U32(Math_Max_U32(image->depth, Math_Max_U32(image->width, image->height)));
	uint32_t minWidth = TinyImageFormat_WidthOfBlock(image->format);
	uint32_t minHeight = TinyImageFormat_HeightOfBlock(image->format);
	uint32_t minDepth = TinyImageFormat_DepthOfBlock(image->format);

	switch (image->format) {
		case TinyImageFormat_PVRTC1_4BPP_UNORM:
		case TinyImageFormat_PVRTC1_4BPP_SRGB:
		case TinyImageFormat_PVRTC2_4BPP_UNORM:
		case TinyImageFormat_PVRTC2_4BPP_SRGB: minWidth = 8;
      minHeight = 8;
      break;
    case TinyImageFormat_PVRTC1_2BPP_UNORM:
    case TinyImageFormat_PVRTC1_2BPP_SRGB:
		case TinyImageFormat_PVRTC2_2BPP_UNORM:
		case TinyImageFormat_PVRTC2_2BPP_SRGB:
    	minWidth = 16;
      minHeight = 8;
      break;
    default:break;
  }

  size_t size = 0;
  int level = maxMipLevels;

  Image_ImageHeader scratch;
  scratch.format = image->format;
  scratch.width = image->width;
  scratch.height = image->height;
  scratch.depth = image->depth;
  scratch.slices = image->slices;

  while (level > 0) {
    size += Image_ByteCountOf(&scratch);

    scratch.width >>= 1;
    scratch.height >>= 1;
    scratch.depth >>= 1;

    if (scratch.width + scratch.height + scratch.depth == 0) {
      break;
    }
    scratch.width = Math_Max_I32(scratch.width, minWidth);
    scratch.height = Math_Max_I32(scratch.height, minHeight);
    scratch.depth = Math_Max_I32(scratch.depth, minDepth);

    level--;
  }
  return size;
}

EXTERN_C Image_ImageHeader const *Image_Create(uint32_t width,
                                               uint32_t height,
                                               uint32_t depth,
                                               uint32_t slices,
                                               TinyImageFormat format,
                                               Memory_Allocator* memoryAllocator) {
	Image_ImageHeader const * image = Image_CreateNoClear(width, height, depth, slices, format, memoryAllocator);
	if (image) {
		memset(Image_RawDataPtr(image), 0, image->dataSize);
	}

	return image;
}

EXTERN_C Image_ImageHeader const *Image_CreateNoClear(uint32_t width,
                                                      uint32_t height,
                                                      uint32_t depth,
                                                      uint32_t slices,
                                                      TinyImageFormat format,
                                                      Memory_Allocator* memoryAllocator) {
	if(width == 0)return nullptr;
	if (format == TinyImageFormat_UNDEFINED) return nullptr;
	if(height == 0) height = 1;
	if(depth == 0) depth = 1;
	if(slices == 0) slices = 1;

	Image_ImageHeader tmp;
	Image_FillHeader(width, height, depth, slices, format, &tmp);

	Image_ImageHeader *image = (Image_ImageHeader *) MALLOC(memoryAllocator, sizeof(Image_ImageHeader) + tmp.dataSize);
	if (!image) { return nullptr; }

	memcpy(image, &tmp, sizeof(Image_ImageHeader));
	image->memoryAllocator = memoryAllocator;

	return image;
}
EXTERN_C void Image_FillHeader(uint32_t width,
                               uint32_t height,
                               uint32_t depth,
                               uint32_t slices,
                               TinyImageFormat format,
                               Image_ImageHeader *header) {


	uint32_t const blockW = TinyImageFormat_WidthOfBlock(format);
	uint32_t const blockH = TinyImageFormat_HeightOfBlock(format);
	uint32_t const blockD = TinyImageFormat_DepthOfBlock(format);

	// smallest sized a block texture can have is the block size
	width = Math_Max_U32(width, blockW);
	height = Math_Max_U32(height, blockH);
	depth = Math_Max_U32(depth, blockD);

	// round up to block size
	width = (width + (blockW-1)) & ~(blockW-1);
	height = (height + (blockH-1)) & ~(blockH-1);
	depth = (depth + (blockD-1)) & ~(blockD-1);

	uint64_t const pixelCount = (uint64_t)width * (uint64_t)height * (uint64_t)depth * (uint64_t)slices;
	uint64_t const blockBitSize = TinyImageFormat_BitSizeOfBlock(format);
	uint64_t const blockPixelCount = (uint64_t)blockW * (uint64_t)blockH * (uint64_t)blockD;
	header->dataSize = (pixelCount * blockBitSize) / (blockPixelCount * 8ULL);

	header->width = width;
	header->height = height;
	header->depth = depth;
	header->slices = slices;
	header->format = format;
	header->flags = 0;
	header->nextType = Image_NT_None;
	header->nextImage = nullptr;
	header->memoryAllocator = nullptr;
}

EXTERN_C Image_ImageHeader const* Image_CreateHeaderOnly(	uint32_t width,
                                                           uint32_t height,
                                                           uint32_t depth,
                                                           uint32_t slices,
                                                           TinyImageFormat format,
                                                           Memory_Allocator* memoryAllocator) {
	Image_ImageHeader *image = (Image_ImageHeader *) MALLOC(memoryAllocator, sizeof(Image_ImageHeader));
	if (!image) { return nullptr; }
	Image_FillHeader(width, height, depth, slices, format, image);
	image->memoryAllocator = memoryAllocator;

	return image;
}

EXTERN_C void Image_Destroy(Image_ImageHeader const *image) {
	if(!image) return;

	// recursively free next in image chain
	if(image->nextType != Image_NT_None) {
		Image_Destroy(image->nextImage);
	}

	MFREE(image->memoryAllocator, (Image_ImageHeader*)image);
}
