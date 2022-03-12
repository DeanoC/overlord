#include "core/core.h"
#include "memory/memory.h"
#include "tiny_image_format/tiny_image_format_base.h"
#include "tiny_image_format/tiny_image_format_query.h"
#include "tiny_image_format/tiny_image_format_decode.h"
#include "tiny_image_format/tiny_image_format_encode.h"
#include "gfx_image/image.h"
#include "gfx_image/copy.h"


void Image_CopyImageChain(Image_ImageHeader const *src, Image_ImageHeader *dst) {
	Image_CopyImage(src, dst);
	if (src->nextType == dst->nextType && src->nextImage && dst->nextImage) {
		Image_CopyImageChain(src->nextImage, dst->nextImage);
	}
}

void Image_CopyImage(Image_ImageHeader const *src, Image_ImageHeader *dst) {
	if (src == dst) {
		return;
	}

	assert(dst->slices == src->slices);
	assert(dst->depth == src->depth);
	assert(dst->height == src->height);
	assert(dst->width == src->width);

	for (uint32_t w = 0u; w < src->slices; ++w) {
		Image_CopySlice(src, w, dst, w);
	}
}

void Image_CopySlice( Image_ImageHeader const *src, uint32_t sw, Image_ImageHeader *dst, uint32_t dw) {
	assert(dst->depth == src->depth);
	assert(dst->height == src->height);
	assert(dst->width == src->width);
	if (dst == src) {
		assert(dw != sw);
	}

	for (uint32_t z = 0u; z < src->depth; ++z) {
		Image_CopyPage(src, z, sw, dst, z, dw);
	}
}

void Image_CopyPage(Image_ImageHeader const *src,
										uint32_t sz, uint32_t sw,
										Image_ImageHeader *dst,
										uint32_t dz, uint32_t dw) {
	assert(dst->height == src->height);
	assert(dst->width == src->width);
	if (dst == src) {
		assert(dz != sz || dw != sw);
	}

	uint32_t const heightOfBlock = TinyImageFormat_HeightOfBlock(src->format);
	for (uint32_t y = 0u; y < src->height / heightOfBlock; ++y) {
		Image_CopyRow(src, y, sz, sw, dst, y, dz, dw);
	}
}

void Image_CopyRow(Image_ImageHeader const *src,
                                  uint32_t sy, uint32_t sz, uint32_t sw,
                                  Image_ImageHeader *dst,
                                  uint32_t dy, uint32_t dz, uint32_t dw) {
	assert(dst->width == src->width);
	if (dst == src) {
		assert(dy != sy || dz != sz || dw != sw);
	}

	// can we do it via the faster float path (more common case)
	if (TinyImageFormat_CanDecodeLogicalPixelsF(src->format) &&
	    TinyImageFormat_CanEncodeLogicalPixelsF(dst->format)) {
		uint32_t const widthOfBlock = TinyImageFormat_WidthOfBlock(src->format);
		uint32_t const pixelCountOfBlock = TinyImageFormat_PixelCountOfBlock(src->format);
		uint64_t const rowBufferSize = (src->width / widthOfBlock) * pixelCountOfBlock * sizeof(float) * 4;

		float* rowBuffer = (float *) STACK_ALLOC(rowBufferSize);

		size_t const srcIndex = Image_CalculateIndex(src, 0, sy, sz, sw);
		size_t const dstIndex = Image_CalculateIndex(dst, 0, dy, dz, dw);
		Image_GetRowAtF(src, rowBuffer, srcIndex);
		Image_SetRowAtF(dst, rowBuffer, dstIndex);

	} else if (TinyImageFormat_CanDecodeLogicalPixelsD(src->format) &&
	           TinyImageFormat_CanEncodeLogicalPixelsD(dst->format)) {
		// this is if we require the double path
		uint32_t const widthOfBlock = TinyImageFormat_WidthOfBlock(src->format);
		uint32_t const pixelCountOfBlock = TinyImageFormat_PixelCountOfBlock(src->format);
		uint64_t const rowBufferSize = (src->width / widthOfBlock) * pixelCountOfBlock * sizeof(double) * 4;

		double* rowBuffer = (double *) STACK_ALLOC(rowBufferSize);

		size_t const srcIndex = Image_CalculateIndex(src, 0, sy, sz, sw);
		size_t const dstIndex = Image_CalculateIndex(dst, 0, dy, dz, dw);
		Image_GetRowAtD(src, rowBuffer, srcIndex);
		Image_SetRowAtD(dst, rowBuffer, dstIndex);
	} else {
		// TODO better error
		// we can't decode and/or encode between these formats so die
		assert(false);
	}
}

void Image_CopyPixel(Image_ImageHeader const *src,
                                    uint32_t sx, uint32_t sy, uint32_t sz, uint32_t sw,
                                    Image_ImageHeader *dst,
                                    uint32_t dx, uint32_t dy, uint32_t dz, uint32_t dw) {
	size_t const srcIndex = Image_CalculateIndex(src, sx, sy, sz, sw);
	size_t const dstIndex = Image_CalculateIndex(src, dx, dy, dz, dw);
	Image_PixelD pixel;
	Image_GetPixelAtD(src, (double*)&pixel, srcIndex);
	Image_SetPixelAtD(dst, (double*)&pixel, dstIndex);
}

Image_ImageHeader *Image_Clone(Image_ImageHeader const *image) {
	Image_ImageHeader * dst = (Image_ImageHeader *) Image_Create(image->width, image->height, image->depth, image->slices, image->format,image->memoryAllocator);
	if (dst == nullptr) {
		return nullptr;
	}
	Image_CopyImage(image, dst);
	if (image->nextType != Image_NT_None) {
		dst->nextImage = Image_Clone(image->nextImage);
		dst->nextType = image->nextType;
	}
	return dst;
}

Image_ImageHeader *Image_CloneStructure(Image_ImageHeader const *image) {
	Image_ImageHeader *dst = (Image_ImageHeader *) Image_Create(image->width, image->height, image->depth, image->slices, image->format,image->memoryAllocator);
	if (dst == nullptr) {
		return nullptr;
	}
	if (image->nextType != Image_NT_None) {
		dst->nextImage = Image_CloneStructure(image->nextImage);
		dst->nextType = image->nextType;
	}
	return dst;
}

Image_ImageHeader *Image_PreciseConvert(Image_ImageHeader const *image, TinyImageFormat const newFormat) {
	Image_ImageHeader * dst = (Image_ImageHeader *) Image_Create(image->width, image->height, image->depth, image->slices, newFormat, image->memoryAllocator);
	if (dst == nullptr) {
		return nullptr;
	}
	Image_CopyImage(image, dst);
	if (image->nextType != Image_NT_None) {
		dst->nextImage = Image_PreciseConvert(image->nextImage, newFormat);
		dst->nextType = image->nextType;
	}
	return dst;
}

Image_ImageHeader *Image_PackMipmaps(Image_ImageHeader const *image) {
	if (Image_HasPackedMipMaps(image))
		return image;

	size_t const numLevels = Image_LinkedImageCountOf(image);
	if (numLevels == 1)
		return image;

	size_t const packedSized = Image_ByteCountOfImageChainOf(image);
	Image_ImageHeader *newImage = (Image_ImageHeader *) MALLOC(image->memoryAllocator, sizeof(Image_ImageHeader) + packedSized);
	Image_FillHeader(image->width, image->height, image->depth, image->slices, image->format, newImage);
	newImage->dataSize = packedSized;
	newImage->flags |= Image_Flag_PackedMipMaps;
	newImage->packedMipMapCount = (uint8_t)numLevels;

	uint8_t *dstPtr = (uint8_t *) Image_RawDataPtr(newImage);
	for (size_t i = 0; i < numLevels; ++i) {
		assert((size_t)(dstPtr - (uint8_t *) Image_RawDataPtr(newImage)) < packedSized);
		Image_ImageHeader const *levelHeader = Image_LinkedImageOf(image, i);
		memcpy(dstPtr, Image_RawDataPtr(levelHeader), levelHeader->dataSize);
		dstPtr += levelHeader->dataSize;
	}
	return newImage;
}