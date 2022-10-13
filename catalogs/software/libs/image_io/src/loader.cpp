#include "core/core.h"
#include "dbg/print.h"
#include "core/compile_time_hash.hpp"
#include "core/utf8.h"
#include "image_io/loader.h"

Image_ImageHeader * Image_Load(VFile_Handle handle, Memory_Allocator* allocator) {
	utf8_int8_t const * name = VFile_GetName(handle);
	utf8_int8_t* pos = utf8rchr(name, '.');
	if(pos) {
		size_t extLen = utf8len(pos);
		utf8_int8_t * ext = (utf8_int8_t*) STACK_ALLOC(extLen);
		utf8ncpy(ext, pos+1, extLen);
		utf8lwr(ext);

		switch (Core::RuntimeHash(extLen-1, (char*)ext)) {
//			case "basis"_hash:
//			case "basisu"_hash:
//				return Image_LoadBasisU(handle);
			case "dds"_hash:
				return Image_LoadDDS(handle, allocator);
			case "pvr"_hash:
				return Image_LoadPVR(handle, allocator);
//			case "exr"_hash:
//				return Image_LoadEXR(handle, allocator);
//			case "hdr"_hash:
//				return Image_LoadHDR(handle, allocator);
			case "jpg"_hash:
			case "jpeg"_hash:
			case "png"_hash:
			case "tga"_hash:
			case "bmp"_hash:
			case "psd"_hash:
			case "gif"_hash:
			case "pic"_hash:
			case "pnm"_hash:
			case "ppm"_hash:
				return Image_LoadLDR(handle, allocator);
//case "ktx2"_hash:
			case "ktx"_hash:
				return Image_LoadKTX(handle, allocator);
			default:
				return nullptr;
		}
	} else {
		return nullptr;
	}
}
