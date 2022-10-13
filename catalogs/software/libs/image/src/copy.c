#include "core/core.h"
#include "memory/memory.h"
#include "tiny_image_format/tiny_image_format_base.h"
#include "tiny_image_format/tiny_image_format_query.h"
#include "tiny_image_format/tiny_image_format_decode.h"
#include "tiny_image_format/tiny_image_format_encode.h"
#include "image/image.h"
#include "image/copy.h"
#include "dbg/print.h"


void Image_CopyImageChain(Image_ImageHeader const *src, Image_ImageHeader *dst) {
	Image_CopyImage(src, dst);
	if (Image_HasNextImageData(src) && Image_HasNextImageData(dst)) {
		Image_CopyImageChain(Image_GetNextImageData(src), Image_GetNextImageData(src));
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
	size_t totalSize = Image_ByteCountOfImageChainOf(image);

	Image_ImageHeader * dst = (Image_ImageHeader *) MALLOC(image->memoryAllocator, totalSize);
	if (dst == nullptr) {
		return nullptr;
	}
	memcpy(dst, image, totalSize);
	return dst;
}

Image_ImageHeader *Image_CloneStructure(Image_ImageHeader const *image) {
	return Image_Clone(image);
}

Image_ImageHeader *Image_PreciseConvert(Image_ImageHeader const *image, TinyImageFormat const newFormat) {
	Image_ImageHeader * dst = (Image_ImageHeader *) Image_Create(image->width, image->height, image->depth, image->slices, newFormat, image->memoryAllocator);
	if (dst == nullptr) {
		debug_print("Image_PreciseConvert failed to create\n");
		return nullptr;
	}
	Image_CopyImage(image, dst);

	if (Image_HasNextImageData(image)) {
		dst = Image_DestructiveJoinImages(dst, Image_PreciseConvert(Image_GetNextImageData(image), newFormat), image->memoryAllocator);
	}
	return dst;
}

