#include "core/core.h"
#include "dbg/assert.h"
#include "core/math.h"
#include "memory/memory.h"
#include "tiny_image_format/tiny_image_format_base.h"
#include "tiny_image_format/tiny_image_format_query.h"
#include "tiny_image_format/tiny_image_format_decode.h"
#include "tiny_image_format/tiny_image_format_encode.h"
#include "image/image.h"
#include "dbg/print.h"

size_t Image_ByteCountOfImageChainOf(Image_ImageHeader const *image) {

	size_t total = Image_ByteCountOf(image);
	if(Image_HasNextImageData(image)) {
		total += Image_ByteCountOfImageChainOf(Image_GetNextImageData(image));
	}
	return total;
}
size_t Image_LinkedImageCountOf(Image_ImageHeader const *image) {
  size_t count = 1;

	while (image && Image_HasNextImageData(image)) {
    count++;
    image = Image_GetNextImageData(image);
  }
  return count;
}

Image_ImageHeader const *Image_LinkedImageOf(Image_ImageHeader const *image, size_t const index) {
  size_t count = 0;

  do {
    if (count == index) {
      return image;
    }
    count++;
	  if(Image_HasNextImageData(image)) {
		  image = Image_GetNextImageData(image);
	  } else {
			return nullptr;
		}
  }  while (image);

return nullptr;
}

bool Image_GetBlocksAtF(Image_ImageHeader const *image, float *pixels, size_t blockCount, size_t index) {
	assert(image);
	assert(pixels);

	if(!TinyImageFormat_CanDecodeLogicalPixelsF(image->format)) return false;

	uint32_t const pixelCount = TinyImageFormat_PixelCountOfBlock(image->format);

	memset(pixels, 0, pixelCount * sizeof(float) * 4);

	uint8_t *pixelPtr = ((uint8_t *) Image_RawDataPtr(image)) +
			index * (TinyImageFormat_BitSizeOfBlock(image->format) / 8);


	TinyImageFormat_DecodeInput input = { .pixel = pixelPtr };
	return TinyImageFormat_DecodeLogicalPixelsF(image->format, &input, (uint32_t)blockCount, pixels);
}

bool Image_SetBlocksAtF(Image_ImageHeader *image, float const *pixels, size_t blockCount, size_t index) {
	assert(image);
	assert(pixels);

	if(!TinyImageFormat_CanEncodeLogicalPixelsF(image->format)) return false;

	uint8_t *pixelPtr = ((uint8_t *) Image_RawDataPtr(image)) +
			index * (TinyImageFormat_BitSizeOfBlock(image->format) / 8);


	TinyImageFormat_EncodeOutput output = { .pixel = pixelPtr };
	return TinyImageFormat_EncodeLogicalPixelsF(image->format, pixels, (uint32_t)blockCount, &output);
}


bool Image_GetPixelAtF(Image_ImageHeader const *image, float *pixel, size_t index) {
	assert(image);
	if(TinyImageFormat_PixelCountOfBlock(image->format) != 1) return false;
	return Image_GetBlocksAtF(image, pixel, 1, index);
}

bool Image_SetPixelAtF(Image_ImageHeader *image, float const *pixel, size_t index) {
	assert(image);
	if(TinyImageFormat_PixelCountOfBlock(image->format) != 1) return false;
	return Image_SetBlocksAtF(image, pixel, 1, index);
}

bool Image_GetRowAtF(Image_ImageHeader const *image, float *pixels, size_t index) {
	assert(image);
	uint32_t const blockWidth = TinyImageFormat_WidthOfBlock(image->format);

	return Image_GetBlocksAtF(image, pixels, image->width / blockWidth, index);
}

bool Image_SetRowAtF(Image_ImageHeader *image, float const *pixels, size_t index) {
	assert(image);

	uint32_t const blockWidth = TinyImageFormat_WidthOfBlock(image->format);
	return Image_SetBlocksAtF(image, pixels, image->width / blockWidth, index);
}


bool Image_GetBlocksAtD(Image_ImageHeader const *image, double *pixels, size_t blockCount, size_t index) {
	assert(image);
	assert(pixels);

	if(!TinyImageFormat_CanDecodeLogicalPixelsD(image->format)) return false;

	uint32_t const pixelCount = TinyImageFormat_PixelCountOfBlock(image->format);

	memset(pixels, 0, pixelCount * sizeof(float) * 4);

	uint8_t *pixelPtr = ((uint8_t *) Image_RawDataPtr(image)) +
			index * (TinyImageFormat_BitSizeOfBlock(image->format) / 8);


	TinyImageFormat_DecodeInput input = { .pixel = pixelPtr };
	return TinyImageFormat_DecodeLogicalPixelsD(image->format, &input, (uint32_t)blockCount, pixels);
}

bool Image_SetBlocksAtD(Image_ImageHeader *image, double const *pixels, size_t blockCount, size_t index) {
	assert(image);
	assert(pixels);

	if(!TinyImageFormat_CanEncodeLogicalPixelsD(image->format)) return false;

	uint8_t *pixelPtr = ((uint8_t *) Image_RawDataPtr(image)) +
			index * (TinyImageFormat_BitSizeOfBlock(image->format) / 8);


	TinyImageFormat_EncodeOutput output = { .pixel = pixelPtr };
	return TinyImageFormat_EncodeLogicalPixelsD(image->format, pixels, (uint32_t)blockCount, &output);
}

bool Image_GetPixelAtD(Image_ImageHeader const *image, double *pixel, size_t index) {
	assert(image);
	if(TinyImageFormat_PixelCountOfBlock(image->format) != 1) return false;
	return Image_GetBlocksAtD(image, pixel, 1, index);
}

bool Image_SetPixelAtD(Image_ImageHeader *image, double const *pixel, size_t index) {
	assert(image);
	if(TinyImageFormat_PixelCountOfBlock(image->format) != 1) return false;
	return Image_SetBlocksAtD(image, pixel, 1, index);
}

bool Image_GetRowAtD(Image_ImageHeader const *image, double *pixels, size_t index) {
	assert(image);
	uint32_t const blockWidth = TinyImageFormat_WidthOfBlock(image->format);

	return Image_GetBlocksAtD(image, pixels, image->width / blockWidth, index);
}

bool Image_SetRowAtD(Image_ImageHeader *image, double const *pixels, size_t index) {
	assert(image);

	uint32_t const blockWidth = TinyImageFormat_WidthOfBlock(image->format);
	return Image_SetBlocksAtD(image, pixels, image->width / blockWidth, index);
}


size_t Image_BytesRequiredForMipMapsOf(Image_ImageHeader const *image) {

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
    scratch.depth = Math_Max_I16(scratch.depth, minDepth);

    level--;
  }
  return size;
}

Image_ImageHeader *Image_Create(uint32_t width,
	                               uint32_t height,
	                               uint32_t depth,
	                               uint32_t slices,
	                               TinyImageFormat format,
	                               Memory_Allocator* memoryAllocator) {
	Image_ImageHeader * image = Image_CreateNoClear(width, height, depth, slices, format, memoryAllocator);
	if (image) {
		memset(Image_RawDataPtr(image), 0, image->dataSizeInBytes - sizeof(Image_ImageHeader));
	}

	return image;
}

Image_ImageHeader *Image_CreateNoClear(uint32_t width,
                                        uint32_t height,
                                        uint32_t depth,
                                        uint32_t slices,
                                        TinyImageFormat format,
                                        Memory_Allocator* memoryAllocator) {
	if(width == 0) {
		debug_print("Image: Width must be > 0\n");
		return nullptr;
	}
	if (format == TinyImageFormat_UNDEFINED) {
		debug_print("Image: Format must not be UNDEFINED\n");
		return nullptr;
	}
	if(height == 0) height = 1;
	if(depth == 0) depth = 1;
	if(slices == 0) slices = 1;

	Image_ImageHeader tmp;
	Image_FillHeader(width, height, depth, slices, format, &tmp);

	Image_ImageHeader *image = (Image_ImageHeader *) MALLOC(memoryAllocator, tmp.dataSizeInBytes);
	if (!image) {
		debug_printf("Image: memory alloc failed for w %i h %i d %i s %i format %s\n", width, height, depth, slices, TinyImageFormat_Name(format));
		return nullptr;
	}

	memcpy(image, &tmp, sizeof(Image_ImageHeader));
	image->memoryAllocator = memoryAllocator;

	return image;
}

uint64_t Image_CalcSize(uint32_t width,
                      uint32_t height,
                      uint32_t depth,
                      uint32_t slices,
                      TinyImageFormat format ) {
	uint32_t const blockW = TinyImageFormat_WidthOfBlock(format);
	uint32_t const blockH = TinyImageFormat_HeightOfBlock(format);
	uint32_t const blockD = TinyImageFormat_DepthOfBlock(format);

	// smallest sized a block texture can have is the block size
	width = Math_Max_U32(width, blockW);
	height = Math_Max_U32(height, blockH);
	depth = Math_Max_U16(depth, blockD);

	// round up to block size
	width = (width + (blockW-1)) & ~(blockW-1);
	height = (height + (blockH-1)) & ~(blockH-1);
	depth = (depth + (blockD-1)) & ~(blockD-1);

	uint64_t const pixelCount = (uint64_t)width * (uint64_t)height * (uint64_t)depth * (uint64_t)slices;
	uint64_t const blockBitSize = TinyImageFormat_BitSizeOfBlock(format);
	uint64_t const blockPixelCount = (uint64_t)blockW * (uint64_t)blockH * (uint64_t)blockD;
	uint64_t totalSize = sizeof(Image_ImageHeader) + ((pixelCount * blockBitSize) / (blockPixelCount * 8ULL));
	totalSize = ((totalSize + 7) & ~7);

	return totalSize;
}

void Image_FillHeader(uint32_t width_,
                       uint32_t height_,
                       uint32_t depth_,
                       uint32_t slices_,
                       TinyImageFormat format_,
                       Image_ImageHeader *header_) {

	header_->dataSizeInBytes = Image_CalcSize(width_, height_, depth_, slices_, format_);
	header_->width = width_;
	header_->height = height_;
	header_->depth = depth_;
	header_->slices = slices_;
	header_->format = format_;
	header_->flags = 0;
	header_->memoryAllocator = nullptr;
}

Image_ImageHeader * Image_CreateHeaderOnly(	uint32_t width,
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

void Image_Destroy(Image_ImageHeader *image) {
	if(!image) return;

	MFREE(image->memoryAllocator, (Image_ImageHeader*)image);
}

size_t Image_MipMapCountOf(Image_ImageHeader const *image) {
	return Image_LinkedImageCountOf(image);
}

Image_ImageHeader * Image_JoinImages(Image_ImageHeader *first_, Image_ImageHeader *second_, Memory_Allocator* memoryAllocator_) {
	Image_ImageHeader *image = (Image_ImageHeader *) MALLOC(memoryAllocator_,
	                                                        first_->dataSizeInBytes + second_->dataSizeInBytes);
	// join the data
	Image_ImageHeader *f = (Image_ImageHeader *) image;
	Image_ImageHeader *s = (Image_ImageHeader *) (((uint8_t *) image) + first_->dataSizeInBytes);
	memcpy(f, first_, first_->dataSizeInBytes);
	memcpy(s, second_, second_->dataSizeInBytes);
	f->memoryAllocator = memoryAllocator_;
	s->memoryAllocator = nullptr;

	// we now need to fix up flags
	Image_ImageHeader *c = f;
	while (Image_HasNextImageData(c)) {
		c = Image_GetNextImageData(c);
	}
	// add the next flag and validate
	c->flags |= Image_Flag_HasNextImageData;
	assert(Image_GetNextImageData(c) == s);

	return image;
}

Image_ImageHeader * Image_DestructiveJoinImages(Image_ImageHeader *first_, Image_ImageHeader *second_, Memory_Allocator* memoryAllocator_) {
	Image_ImageHeader *image = Image_JoinImages(first_, second_, memoryAllocator_);
	Image_Destroy(first_);
	Image_Destroy(second_);

	return image;
}
