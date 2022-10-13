#include "core/core.h"
#include "dbg/print.h"
#include "library_defines/library_defines.h"

#if IKUY_HAVE_LIB_RESOURCE_BUNDLE_WRITER == 1
#include "image/resource_writer.hpp"
#include "image/image.h"
#include "tiny_stl/string.hpp"
#include "data_binify/write_helper.hpp"

void ImageChunkWriter(void * userData_, Binify::WriteHelper& helper) {
	if(userData_ == nullptr) {
		helper.addEnum("Image_FlagBits");
		helper.addEnumValue("Image_FlagBits", "Cubemap", Image_Flag_Cubemap);
		helper.addEnumValue("Image_FlagBits", "HeaderOnly", Image_Flag_HeaderOnly);
		helper.addEnumValue("Image_FlagBits", "HasNextImageData", Image_Flag_HasNextImageData);
		helper.addEnumValue("Image_FlagBits", "CLUT", Image_Flag_CLUT);
		return;
	}

	auto image = (Image_ImageHeader const * const) userData_;
	assert((image->dataSizeInBytes & 0x7) == 0);
	while(image != nullptr) {
		helper.assignAddressAsLabel(image, true, "Image Label");
		helper.writeNullPtr("Where the allocator pointer lives");
		helper.writeAs<uint64_t>(image->dataSizeInBytes, "Image size in bytes");
		helper.writeAs<uint32_t>(image->width, "Width");
		helper.writeAs<uint32_t>(image->height, "Height");
		helper.writeAs<uint16_t>(image->depth, "Depth");
		helper.writeAs<uint16_t>(image->slices, "Slices");
		helper.writeFlagsAs<uint16_t>("Image_FlagBits", image->flags, "// Image Flags");
		helper.writeAs<uint16_t>(image->format, TinyImageFormat_Name(image->format));
		helper.comment("Image Data");
		helper.writeByteArray((uint8_t *) Image_RawDataPtr(image), image->dataSizeInBytes - sizeof(Image_ImageHeader));
		helper.align();
		if(Image_HasNextImageData(image)) {
			image = (Image_ImageHeader const * const)(((uint8_t*)image) + image->dataSizeInBytes);
		} else {
			image = nullptr;
		}
	}
}

#else
#endif