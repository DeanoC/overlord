#include "core/core.h"
#include "dbg/print.h"
#include "library_defines/library_defines.h"

#if IKUY_HAVE_LIB_RESOURCE_BUNDLE_WRITER == 1
#include "data_binify/write_helper.hpp"
#include "gfx2d/sprite.hpp"
#include "image/image.h"

void SpriteDataChunkWriter(void * userData_, Binify::WriteHelper& helper) {
	// no setup phase needed
	if(!userData_) return;

	using namespace Gfx2d;
	auto sprite = (SpriteData const *const) userData_;
	helper.useAddressAsLabel(sprite->image);
	helper.addString(sprite->handlerName);
	helper.align(8);
	helper.writeAs<int16_t>(sprite->spriteSheetCoord.x, "SpriteSheetCoord");
	helper.writeAs<int16_t>(sprite->spriteSheetCoord.y);
	helper.writeAs<int16_t>(sprite->spriteDimension.w, "spriteDimension");
	helper.writeAs<int16_t>(sprite->spriteDimension.h);
	helper.writeAs<int16_t>(sprite->spritePivotPoint.x, "spritePivotPoint");
	helper.writeAs<int16_t>(sprite->spritePivotPoint.y);

	static_assert(sizeof(TinyImageFormat) == 1);
}

#endif

