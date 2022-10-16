#include "core/core.h"
#include "dbg/print.h"
#include "memory/memory.h"
#include "vfile/vfile.h"
#include "tiny_image_format/tiny_image_format_base.h"
#include "image/image.h"

#include "stb_image_write.h"
#define TINYKTX_HAVE_MEMCPY
#include "tiny_ktx.h"
#define TINYDDS_HAVE_MEMCPY
#include "tiny_dds.h"

// TODO EXR
//#include "syoyo/tiny_exr.hpp"


static void stbIoCallbackWrite(void *user, void *data, int size) {
	VFile_Handle handle = (VFile_Handle) user;
	VFile_Write(handle, data, size);
}

bool ImageIO_SaveAsTGA(Image_ImageHeader *image, VFile_Handle handle) {
	if (!handle) {
		return false;
	}
	void *src = Image_RawDataPtr(image);

	switch (image->format) {
		case TinyImageFormat_R8_UNORM:
		case TinyImageFormat_R8_SRGB:
			return 0 != stbi_write_tga_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 1, src);
		case TinyImageFormat_R8G8_UNORM:
		case TinyImageFormat_R8G8_SRGB:
			return 0 != stbi_write_tga_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 2, src);
		case TinyImageFormat_R8G8B8_UNORM:
		case TinyImageFormat_R8G8B8_SRGB:
			return 0 != stbi_write_tga_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 3, src);
		case TinyImageFormat_R8G8B8A8_UNORM:
		case TinyImageFormat_R8G8B8A8_SRGB:
			return 0 != stbi_write_tga_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 4, src);
		default: {
			return false;
		}
	}
}

bool ImageIO_SaveAsBMP(Image_ImageHeader *image, VFile_Handle handle) {
	if (!handle) {
		return false;
	}
	void *src = Image_RawDataPtr(image);

	switch (image->format) {
		case TinyImageFormat_R8_UNORM:
		case TinyImageFormat_R8_SRGB:
			return 0 != stbi_write_bmp_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 1, src);
		case TinyImageFormat_R8G8_UNORM:
		case TinyImageFormat_R8G8_SRGB:
			return 0 != stbi_write_bmp_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 2, src);
		case TinyImageFormat_R8G8B8_UNORM:
		case TinyImageFormat_R8G8B8_SRGB:
			return 0 != stbi_write_bmp_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 3, src);
		case TinyImageFormat_R8G8B8A8_UNORM:
		case TinyImageFormat_R8G8B8A8_SRGB:
			return 0 != stbi_write_bmp_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 4, src);
		default: {
			return false;
		}
	}
}

bool ImageIO_SaveAsPNG(Image_ImageHeader *image, VFile_Handle handle) {
	if (!handle) {
		return false;
	}
	void *src = Image_RawDataPtr(image);

	switch (image->format) {
		case TinyImageFormat_R8_UNORM:
		case TinyImageFormat_R8_SRGB:
			return 0 != stbi_write_png_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 1, src, 0);
		case TinyImageFormat_R8G8_UNORM:
		case TinyImageFormat_R8G8_SRGB:
			return 0 != stbi_write_png_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 2, src, 0);
		case TinyImageFormat_R8G8B8_UNORM:
		case TinyImageFormat_R8G8B8_SRGB:
			return 0 != stbi_write_png_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 3, src, 0);
		case TinyImageFormat_R8G8B8A8_UNORM:
		case TinyImageFormat_R8G8B8A8_SRGB:
			return 0 != stbi_write_png_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 4, src, 0);
		default: return false;
	}
}

bool ImageIO_SaveAsJPG(Image_ImageHeader *image, VFile_Handle handle) {


	if (!handle) {
		return false;
	}
	void *src = Image_RawDataPtr(image);

	switch (image->format) {
		case TinyImageFormat_R8_UNORM:
		case TinyImageFormat_R8_SRGB:
			return 0 != stbi_write_jpg_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 1, src, 0);
		case TinyImageFormat_R8G8_UNORM:
		case TinyImageFormat_R8G8_SRGB:
			return 0 != stbi_write_jpg_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 2, src, 0);
		case TinyImageFormat_R8G8B8_UNORM:
		case TinyImageFormat_R8G8B8_SRGB:
			return 0 != stbi_write_jpg_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 3, src, 0);
		case TinyImageFormat_R8G8B8A8_UNORM:
		case TinyImageFormat_R8G8B8A8_SRGB:
			return 0 != stbi_write_jpg_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 4, src, 0);
		default:
			return false;
	}
}
/*
bool ImageIO_SaveAsHDR(Image_ImageHeader *image, VFile_Handle handle) {

	if (!handle) {
		return false;
	}
	float const *src = (float const *) Image_RawDataPtr(image);

	switch (image->format) {
		case TinyImageFormat_R32_SFLOAT:
			return 0 != stbi_write_hdr_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 1, src);
		case TinyImageFormat_R32G32_SFLOAT:
			return 0 != stbi_write_hdr_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 2, src);
		case TinyImageFormat_R32G32B32_SFLOAT:
			return 0 != stbi_write_hdr_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 3, src);
		case TinyImageFormat_R32G32B32A32_SFLOAT:
			return 0 != stbi_write_hdr_to_func(&stbIoCallbackWrite, handle,
			                                   image->width, image->height, 4, src);
		default:
			return false;
	}
}
*/


static void tinyktxCallbackError(void *user, char const *msg) {
	debug_printf("Tiny_Ktx ERROR: %s", msg);
}

static void tinyktxCallbackWrite(void *user, void const *data, size_t size) {
	VFile_Handle handle = (VFile_Handle) user;
	VFile_Write(handle, data, size);
}

bool ImageIO_SaveAsKTX(Image_ImageHeader *image, VFile_Handle handle) {

	TinyKtx_WriteCallbacks callback = {
			&tinyktxCallbackError,
			&tinyktxCallbackWrite,
	};

	TinyKtx_Format fmt = TinyImageFormat_ToTinyKtxFormat(image->format);
	if(fmt == TKTX_UNDEFINED) return false;

	uint32_t numMipmaps = (uint32_t)Image_MipMapCountOf(image);

	uint32_t mipmapsizes[TINYKTX_MAX_MIPMAPLEVELS];
	void const* mipmaps[TINYKTX_MAX_MIPMAPLEVELS];
	memset(mipmapsizes, 0, sizeof(uint32_t)*TINYKTX_MAX_MIPMAPLEVELS);
	memset(mipmaps, 0, sizeof(void const*)*TINYKTX_MAX_MIPMAPLEVELS);

	for(size_t i = 0; i < numMipmaps; ++i) {
		mipmapsizes[i] = (uint32_t) Image_LinkedImageOf(image, i)->dataSizeInBytes - sizeof(Image_ImageHeader);
		mipmaps[i] = Image_RawDataPtr(Image_LinkedImageOf(image, i));
	}

	return TinyKtx_WriteImage(&callback,
	                          handle,
	                          image->width,
	                          image->height,
	                          image->depth,
	                          image->slices,
	                          numMipmaps,
	                          fmt,
	                          Image_IsCubemap(image),
	                          mipmapsizes,
	                          mipmaps );
}


bool ImageIO_CanSaveAsTGA(Image_ImageHeader const *image) {
	if(!Image_Is1D(image) || !Image_Is2D(image)) return false;

	switch (image->format) {
		case TinyImageFormat_R8_UNORM:
		case TinyImageFormat_R8_SRGB:
		case TinyImageFormat_R8G8_UNORM:
		case TinyImageFormat_R8G8_SRGB:
		case TinyImageFormat_R8G8B8_UNORM:
		case TinyImageFormat_R8G8B8_SRGB:
		case TinyImageFormat_R8G8B8A8_UNORM:
		case TinyImageFormat_R8G8B8A8_SRGB:
			return true;
		default: return false;
	}
}

bool ImageIO_CanSaveAsBMP(Image_ImageHeader const *image) {
	if(!Image_Is1D(image) || !Image_Is2D(image)) return false;

	switch (image->format) {
		case TinyImageFormat_R8_UNORM:
		case TinyImageFormat_R8_SRGB:
		case TinyImageFormat_R8G8_UNORM:
		case TinyImageFormat_R8G8_SRGB:
		case TinyImageFormat_R8G8B8_UNORM:
		case TinyImageFormat_R8G8B8_SRGB:
		case TinyImageFormat_R8G8B8A8_UNORM:
		case TinyImageFormat_R8G8B8A8_SRGB:
			return true;
		default: return false;
	}
}
bool ImageIO_CanSaveAsPNG(Image_ImageHeader const *image) {
	if(!Image_Is1D(image) || !Image_Is2D(image)) return false;

	switch (image->format) {
		case TinyImageFormat_R8_UNORM:
		case TinyImageFormat_R8_SRGB:
		case TinyImageFormat_R8G8_UNORM:
		case TinyImageFormat_R8G8_SRGB:
		case TinyImageFormat_R8G8B8_UNORM:
		case TinyImageFormat_R8G8B8_SRGB:
		case TinyImageFormat_R8G8B8A8_UNORM:
		case TinyImageFormat_R8G8B8A8_SRGB:
			return true;
		default: return false;
	}

}
bool ImageIO_CanSaveAsJPG(Image_ImageHeader const *image) {
	if(!Image_Is1D(image) || !Image_Is2D(image)) return false;

	switch (image->format) {
		case TinyImageFormat_R8_UNORM:
		case TinyImageFormat_R8_SRGB:
		case TinyImageFormat_R8G8_UNORM:
		case TinyImageFormat_R8G8_SRGB:
		case TinyImageFormat_R8G8B8_UNORM:
		case TinyImageFormat_R8G8B8_SRGB:
		case TinyImageFormat_R8G8B8A8_UNORM:
		case TinyImageFormat_R8G8B8A8_SRGB:
			return true;
		default:
			return false;
	}
}

bool ImageIO_CanSaveAsKTX(Image_ImageHeader const *image) {
	if(Image_Is3D(image) && Image_IsArray(image)) return false;

	TinyKtx_Format fmt = TinyImageFormat_ToTinyKtxFormat(image->format);
	return !(fmt == TKTX_UNDEFINED);

}
bool ImageIO_CanSaveAsHDR(Image_ImageHeader const *image) {
	if(!Image_Is1D(image) || !Image_Is2D(image)) return false;

	switch (image->format) {
		case TinyImageFormat_R32_SFLOAT:
		case TinyImageFormat_R32G32_SFLOAT:
		case TinyImageFormat_R32G32B32_SFLOAT:
		case TinyImageFormat_R32G32B32A32_SFLOAT:
			return true;
		default:
			return false;
	}
}

bool ImageIO_SaveAsDDS(Image_ImageHeader *image, VFile_Handle handle) {

	TinyDDS_WriteCallbacks callback = {
			&tinyktxCallbackError,
			&tinyktxCallbackWrite,
	};

	TinyDDS_Format fmt = TinyImageFormat_ToTinyDDSFormat(image->format);
	if(fmt == TDDS_UNDEFINED) return false;

	uint32_t numMipmaps = (uint32_t)Image_MipMapCountOf(image);

	uint32_t mipmapsizes[TINYDDS_MAX_MIPMAPLEVELS];
	void const* mipmaps[TINYDDS_MAX_MIPMAPLEVELS];
	memset(mipmapsizes, 0, sizeof(uint32_t)*TINYDDS_MAX_MIPMAPLEVELS);
	memset(mipmaps, 0, sizeof(void const*)*TINYDDS_MAX_MIPMAPLEVELS);

	for(size_t i = 0; i < numMipmaps; ++i) {
		mipmapsizes[i] = (uint32_t) Image_LinkedImageOf(image, i)->dataSizeInBytes - sizeof(Image_ImageHeader);
		mipmaps[i] = Image_RawDataPtr(Image_LinkedImageOf(image, i));
	}

	return TinyDDS_WriteImage(&callback,
	                          handle,
	                          image->width,
	                          image->height,
	                          image->depth,
	                          image->slices,
	                          numMipmaps,
	                          fmt,
	                          Image_IsCubemap(image),
	                          true,
	                          mipmapsizes,
	                          mipmaps );
}

bool ImageIO_CanSaveAsDDS(Image_ImageHeader const *image) {
	if(Image_Is3D(image) && Image_IsArray(image)) return false;

	TinyDDS_Format fmt = TinyImageFormat_ToTinyDDSFormat(image->format);
	// this isn't completely correct... if its an array only dx10 formats
	// can be used. Need to add support to check for this
	return !(fmt == TDDS_UNDEFINED);
}

