//
// Created by deano on 3/25/22.
//
#include "core/core.h"
#include "library_defines/library_defines.h"

#if IKUY_HAVE_LIB_LUAU == 1 || IKUY_HAVE_LIB_LUAU_VM
#include "multi_core/core_local.h"
#include "image/create.h"
#include "image/copy.h"

#include "host_os/osvfile.h"

#if IKUY_HAVE_LIB_IMAGE_IO == 1
#include "image_io/saver.h"
#include "image_io/loader.h"
#endif

#include "lua.h"
#include "lualib.h"
#include "luau_utils/utils.hpp"

static char const MetaName[] = "ikuy.Image";
static CORE_LOCAL(Memory_Allocator * , luaAllocator);

static void imageud_gc (void *image_) {
	auto image = *(Image_ImageHeader **)image_;
	if (image) Image_Destroy(image);
}

// create the null image user data return on the lua state
static Image_ImageHeader const** imageud_create(lua_State *L) {
	// allocate a pointer and push it onto the stack
	auto ud = (Image_ImageHeader const**)lua_newuserdatadtor(L, sizeof(Image_ImageHeader*), &imageud_gc);
	if(ud == nullptr) return nullptr;

	*ud = nullptr;
	luaL_getmetatable(L, MetaName);
	lua_setmetatable(L, -2);
	return ud;
}


static int width(lua_State * L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, image->width);
	return 1;
}

static int height(lua_State * L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, image->height);
	return 1;
}
static int depth(lua_State * L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, image->depth);
	return 1;
}
static int slices(lua_State * L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, image->slices);
	return 1;
}

static int format(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushstring(L, TinyImageFormat_Name(image->format));
	return 1;
}

static int flags(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_createtable(L, 0, 2);
	lua_pushboolean(L, image->flags & Image_Flag_Cubemap);
	lua_setfield(L, -2, "Cubemap");
	lua_pushboolean(L, image->flags & Image_Flag_HeaderOnly);
	lua_setfield(L, -2, "HeaderOnly");
	return 1;
}
static int dimensions(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");

	lua_pushinteger(L, image->width);
	lua_pushinteger(L, image->height);
	lua_pushinteger(L, image->depth);
	lua_pushinteger(L, image->slices);
	return 4;
}

static int getPixelAt(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	int64_t index = luaL_checkinteger(L, 2);
	double pixel[4];
	Image_GetPixelAtD(image, (double*)&pixel, index);

	lua_pushnumber(L, pixel[0]); // r
	lua_pushnumber(L, pixel[1]); // g
	lua_pushnumber(L, pixel[2]); // b
	lua_pushnumber(L, pixel[3]); // a

	return 4;
}

static int setPixelAt(lua_State *L) {
	auto image = *(Image_ImageHeader **)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	int64_t index = luaL_checkinteger(L, 2);

	double pixel[4];
	pixel[0] = luaL_checknumber(L, 3); // r
	pixel[1] = luaL_checknumber(L, 4); // g
	pixel[2] = luaL_checknumber(L, 5); // b
	pixel[3] = luaL_checknumber(L, 6); // a

	Image_SetPixelAtD(image, pixel, index);

	return 0;
}

static int is1D(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushboolean(L, Image_Is1D(image));
	return 1;
}

static int is2D(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushboolean(L, Image_Is2D(image));
	return 1;
}

static int is3D(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushboolean(L, Image_Is3D(image));
	return 1;
}

static int isArray(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushboolean(L, Image_IsArray(image));
	return 1;
}

static int isCubemap(lua_State *L) {
	Image_ImageHeader* image = *(Image_ImageHeader**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushboolean(L, Image_IsCubemap(image));
	return 1;
}

static int pixelCount(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_PixelCountOf(image));
	return 1;
}
static int pixelCountPerSlice(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_PixelCountPerSliceOf(image));
	return 1;
}

static int pixelCountPerPage(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_PixelCountPerPageOf(image));
	return 1;
}

static int pixelCountPerRow(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_PixelCountPerRowOf(image));
	return 1;
}

static int byteCount(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_ByteCountOf(image));
	return 1;
}

static int byteCountPerSlice(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_ByteCountPerSliceOf(image));
	return 1;
}


static int byteCountPerPage(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_ByteCountPerPageOf(image));
	return 1;
}


static int byteCountPerRow(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_ByteCountPerRowOf(image));
	return 1;
}

static int byteCountOfImageChain(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_ByteCountOfImageChainOf(image));
	return 1;
}

static int bytesRequiredForMipMaps(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_BytesRequiredForMipMapsOf(image));
	return 1;
}

static int calculateIndex(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	int64_t x = luaL_checkinteger(L, 2);
	int64_t y = luaL_checkinteger(L, 3);
	int64_t z = luaL_checkinteger(L, 4);
	int64_t s = luaL_checkinteger(L, 5);

	lua_pushinteger(L, Image_CalculateIndex(image, (uint32_t)x, (uint32_t)y, (uint32_t)z, (uint32_t)s));
	return 1;
}

static int copy(lua_State *L) {
	auto src = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	auto dst = *(Image_ImageHeader **)luaL_checkudata(L, 2, MetaName);
	LUA_ASSERT(src, L, "image is NIL");
	LUA_ASSERT(dst, L, "image is NIL");

	Image_CopyImage(src, dst);
	return 0;
}

static int copySlice(lua_State *L) {
	auto src = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	int64_t sw = luaL_checkinteger(L, 2);
	auto dst = *(Image_ImageHeader **)luaL_checkudata(L, 3, MetaName);
	int64_t dw = luaL_checkinteger(L, 4);
	LUA_ASSERT(src, L, "image is NIL");
	LUA_ASSERT(dst, L, "image is NIL");

	Image_CopySlice(src, (uint32_t)sw, dst, (uint32_t)dw);
	return 0;
}

static int copyPage(lua_State *L) {
	auto src = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	int64_t sz = luaL_checkinteger(L, 2);
	int64_t sw = luaL_checkinteger(L, 3);
	auto dst = *(Image_ImageHeader **)luaL_checkudata(L, 4, MetaName);
	int64_t dz = luaL_checkinteger(L, 5);
	int64_t dw = luaL_checkinteger(L, 6);
	LUA_ASSERT(src, L, "image is NIL");
	LUA_ASSERT(dst, L, "image is NIL");

	Image_CopyPage(src, (uint32_t)sz, (uint32_t)sw, dst, (uint32_t)dz, (uint32_t)dw);
	return 0;
}

static int copyRow(lua_State *L) {
	auto src = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	int64_t sy = luaL_checkinteger(L, 2);
	int64_t sz = luaL_checkinteger(L, 3);
	int64_t sw = luaL_checkinteger(L, 4);
	auto dst = *(Image_ImageHeader **)luaL_checkudata(L, 5, MetaName);
	int64_t dy = luaL_checkinteger(L, 6);
	int64_t dz = luaL_checkinteger(L, 7);
	int64_t dw = luaL_checkinteger(L, 8);

	LUA_ASSERT(src, L, "image is NIL");
	LUA_ASSERT(dst, L, "image is NIL");

	Image_CopyRow(src, (uint32_t)sy, (uint32_t)sz, (uint32_t)sw, dst, (uint32_t)dy, (uint32_t)dz, (uint32_t)dw);
	return 0;
}

static int copyPixel(lua_State *L) {
	auto src = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	int64_t sx = luaL_checkinteger(L, 2);
	int64_t sy = luaL_checkinteger(L, 3);
	int64_t sz = luaL_checkinteger(L, 4);
	int64_t sw = luaL_checkinteger(L, 5);
	auto dst = *(Image_ImageHeader **)luaL_checkudata(L, 6, MetaName);
	int64_t dx = luaL_checkinteger(L, 7);
	int64_t dy = luaL_checkinteger(L, 8);
	int64_t dz = luaL_checkinteger(L, 9);
	int64_t dw = luaL_checkinteger(L, 10);

	LUA_ASSERT(src, L, "image is NIL");
	LUA_ASSERT(dst, L, "image is NIL");

	Image_CopyPixel(src, (uint32_t)sx, (uint32_t)sy, (uint32_t)sz, (uint32_t)sw, dst, (uint32_t)dx, (uint32_t)dy, (uint32_t)dz, (uint32_t)dw);
	return 0;
}

static int linkedImageCount(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_LinkedImageCountOf(image));
	return 1;
}

static int linkedImage(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	int64_t index = luaL_checkinteger(L, 2);
	auto ud = imageud_create(L);
	*ud = Image_LinkedImageOf(image, index);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	int64_t s = luaL_checkinteger(L, 4);
	char const* fmt = luaL_checkstring(L, 5);

	auto ud = imageud_create(L);
	*ud = Image_Create((uint32_t)w, (uint32_t)h, (uint32_t)d, (uint32_t)s, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int createNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	int64_t s = luaL_checkinteger(L, 4);
	char const* fmt = luaL_checkstring(L, 5);

	auto ud = imageud_create(L);
	*ud = Image_CreateNoClear((uint32_t)w, (uint32_t)h, (uint32_t)d, (uint32_t)s, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create1D(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	char const* fmt = luaL_checkstring(L, 2);

	auto ud = imageud_create(L);
	*ud = Image_Create1D((uint32_t)w,TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create1DNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	char const* fmt = luaL_checkstring(L, 2);

	auto ud = imageud_create(L);
	*ud = Image_Create1DNoClear((uint32_t)w, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create1DArray(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t s = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_Create1DArray((uint32_t)w, (uint32_t)s, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create1DArrayNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t s = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_Create1DArrayNoClear((uint32_t)w, (uint32_t)s, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create2D(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_Create2D((uint32_t)w, (uint32_t)h, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}
static int create2DNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_Create2DNoClear((uint32_t)w, (uint32_t)h, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create2DArray(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t s = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_Create2DArray((uint32_t)w, (uint32_t)h, (uint32_t)s, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create2DArrayNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t s = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_Create2DArrayNoClear((uint32_t)w, (uint32_t)h, (uint32_t)s, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create3D(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_Create3D((uint32_t)w, (uint32_t)h, (uint32_t)d, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create3DNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_Create3DNoClear((uint32_t)w, (uint32_t)h, (uint32_t)d, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create3DArray(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	int64_t s = luaL_checkinteger(L, 4);
	char const* fmt = luaL_checkstring(L, 5);

	auto ud = imageud_create(L);
	*ud = Image_Create3DArray((uint32_t)w, (uint32_t)h, (uint32_t)d, (uint32_t)s, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}
static int create3DArrayNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	int64_t s = luaL_checkinteger(L, 4);
	char const* fmt = luaL_checkstring(L, 5);

	auto ud = imageud_create(L);
	*ud = Image_Create3DArrayNoClear((uint32_t)w, (uint32_t)h, (uint32_t)d, (uint32_t)s, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}
static int createCubemap(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_CreateCubemap((uint32_t)w, (uint32_t)h, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int createCubemapNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_CreateCubemapNoClear((uint32_t)w, (uint32_t)h, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int createCubemapArray(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t s = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_CreateCubemapArray((uint32_t)w, (uint32_t)h, (uint32_t)s, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int createCubemapArrayNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t s = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_CreateCubemapArrayNoClear((uint32_t)w, (uint32_t)h, (uint32_t)s, TinyImageFormat_FromName(fmt), luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}


static int clone(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	auto ud = imageud_create(L);
	*ud = Image_Clone(image);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int cloneStructure(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	auto ud = imageud_create(L);
	*ud = Image_CloneStructure(image);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int preciseConvert(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	auto ud = imageud_create(L);
	TinyImageFormat to = TinyImageFormat_FromName(luaL_checkstring(L,2));
	if(to != TinyImageFormat_UNDEFINED) *ud = Image_PreciseConvert(image, to);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}
/*
static int createMipMapChain(lua_State * L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool generateFromImage = lua_isnil(L, 2) ? true : (bool)lua_toboolean(L, 2);
	Image_CreateMipMapChain(image,generateFromImage);
	return 0;
}

static int fastConvert(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_FastConvert(image, TinyImageFormat_FromName(luaL_checkstring(L,2)), allowInPlace);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC1(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC1(image, nullptr, nullptr, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC2(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC2(image, nullptr, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC3(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC3(image, nullptr, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC4(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC4(image, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC5(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC5(image, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC6H(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC6H(image, nullptr, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC7(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC7(image, nullptr, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

*/
#if IKUY_HAVE_LIB_IMAGE_IO == 1

static int load(lua_State * L) {
	char const* filename = luaL_checkstring(L, 1);

	VFile_Handle fh = Os_VFileFromFile(filename, Os_FM_ReadBinary, luaAllocator);
	if(!fh) {
		lua_pushnil(L);
		lua_pushboolean(L, false);
		return 2;
	}

	auto ud = imageud_create(L);
	*ud = Image_Load(fh, luaAllocator);
	lua_pushboolean(L, *ud != nullptr);
	VFile_Close(fh);

	return 2;
}

static int saveAsDDS(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile_Handle fh = Os_VFileFromFile(filename, Os_FM_WriteBinary, luaAllocator);
	if(!fh) {
		lua_pushboolean(L, false);
		return 1;
	}
	Image_ImageHeader* image = *(Image_ImageHeader**)ud;
	bool ret = ImageIO_SaveAsDDS(image, fh);
	lua_pushboolean(L, ret);
	VFile_Close(fh);
	return 1;
}

static int saveAsTGA(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile_Handle fh = Os_VFileFromFile(filename, Os_FM_WriteBinary, luaAllocator);
	if(!fh) {
		lua_pushboolean(L, false);
		return 1;
	}
	Image_ImageHeader* image = *(Image_ImageHeader**)ud;
	bool ret = ImageIO_SaveAsTGA(image, fh);
	lua_pushboolean(L, ret);
	VFile_Close(fh);
	return 1;
}
static int saveAsBMP(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile_Handle fh = Os_VFileFromFile(filename, Os_FM_WriteBinary, luaAllocator);
	if(!fh) {
		lua_pushboolean(L, false);
		return 1;
	}
	auto image = *(Image_ImageHeader **)ud;
	bool ret = ImageIO_SaveAsBMP(image, fh);
	lua_pushboolean(L, ret);
	VFile_Close(fh);
	return 1;
}
static int saveAsPNG(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile_Handle fh = Os_VFileFromFile(filename, Os_FM_WriteBinary, luaAllocator);
	if(!fh) {
		lua_pushboolean(L, false);
		return 1;
	}
	auto image = *(Image_ImageHeader **)ud;
	bool ret = ImageIO_SaveAsPNG(image, fh);
	lua_pushboolean(L, ret);
	VFile_Close(fh);
	return 1;
}

static int saveAsJPG(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile_Handle fh = Os_VFileFromFile(filename, Os_FM_WriteBinary, luaAllocator);
	if(!fh) {
		lua_pushboolean(L, false);
		return 1;
	}
	auto image = *(Image_ImageHeader **)ud;
	bool ret = ImageIO_SaveAsJPG(image, fh);
	lua_pushboolean(L, ret);
	VFile_Close(fh);
	return 1;
}

static int saveAsKTX(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile_Handle fh = Os_VFileFromFile(filename, Os_FM_WriteBinary, luaAllocator);
	if(!fh) {
		lua_pushboolean(L, false);
		return 1;
	}
	auto image = *(Image_ImageHeader **)ud;
	bool ret = ImageIO_SaveAsKTX(image, fh);
	lua_pushboolean(L, ret);
	VFile_Close(fh);
	return 1;
}


static int canSaveAsHDR(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = false; // TODO ImageIO_CanSaveAsHDR(image);
	lua_pushboolean(L, ret);
	return 1;
}

/*static int saveAsHDR(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile_Handle fh = Os_VFileFromFile(filename, Os_FM_WriteBinary, luaAllocator);
	if(!fh) {
		VFile_Close(fh);
		return 0;
	}
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = ImageIO_SaveAsHDR(image, fh);
	lua_pushboolean(L, ret);
	VFile_Close(fh);
	return 1;
}*/

static int canSaveAsDDS(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	Image_ImageHeader* image = *(Image_ImageHeader**)ud;
	bool ret = ImageIO_CanSaveAsDDS(image);
	lua_pushboolean(L, ret);
	return 1;
}

static int canSaveAsTGA(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	Image_ImageHeader* image = *(Image_ImageHeader**)ud;
	bool ret = ImageIO_CanSaveAsTGA(image);
	lua_pushboolean(L, ret);
	return 1;
}
static int canSaveAsBMP(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = ImageIO_CanSaveAsBMP(image);
	lua_pushboolean(L, ret);
	return 1;
}
static int canSaveAsPNG(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = ImageIO_CanSaveAsPNG(image);
	lua_pushboolean(L, ret);
	return 1;
}

static int canSaveAsJPG(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);

	auto image = *(Image_ImageHeader const**)ud;
	bool ret = ImageIO_CanSaveAsJPG(image);
	lua_pushboolean(L, ret);
	return 1;
}

static int canSaveAsKTX(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = ImageIO_CanSaveAsKTX(image);
	lua_pushboolean(L, ret);
	return 1;
}
#endif // IKUY_HAVE_LIB_IMAGE_IO == 1

int LuaImage_Open(lua_State* L, Memory_Allocator* allocator) {
	static const struct luaL_Reg imageObj [] = {
#define ENTRY(name) { #name, &(name) }
			ENTRY(width),
			ENTRY(height),
			ENTRY(depth),
			ENTRY(slices),
			ENTRY(dimensions),
			ENTRY(format),
			ENTRY(flags),
			ENTRY(getPixelAt),
			ENTRY(setPixelAt),
			ENTRY(copy),
			ENTRY(copySlice),
			ENTRY(copyPage),
			ENTRY(copyRow),
			ENTRY(copyPixel),
			ENTRY(is1D),
			ENTRY(is2D),
			ENTRY(is3D),
			ENTRY(isArray),
			ENTRY(isCubemap),
			ENTRY(calculateIndex ),
			ENTRY(pixelCount ),
			ENTRY(pixelCountPerSlice ),
			ENTRY(pixelCountPerPage ),
			ENTRY(pixelCountPerRow ),
			ENTRY(byteCount ),
			ENTRY(byteCountPerSlice ),
			ENTRY(byteCountPerPage ),
			ENTRY(byteCountPerRow ),
			ENTRY(linkedImageCount),
			ENTRY(linkedImage),
			ENTRY(byteCountOfImageChain),
			ENTRY(bytesRequiredForMipMaps),
			ENTRY(clone),
			ENTRY(cloneStructure),
			ENTRY(preciseConvert),
			/*
						{"createMipMapChain", &createMipMapChain},
						{"fastConvert", &fastConvert},

						{"compressAMDBC1", &compressAMDBC1},
						{"compressAMDBC2", &compressAMDBC2},
						{"compressAMDBC3", &compressAMDBC3},
						{"compressAMDBC4", &compressAMDBC4},
						{"compressAMDBC5", &compressAMDBC5},
						{"compressAMDBC6H", &compressAMDBC6H},
						{"compressAMDBC7", &compressAMDBC7},
			*/
#if IKUY_HAVE_LIB_IMAGE_IO == 1
			ENTRY(canSaveAsTGA),
			ENTRY(saveAsTGA),
			ENTRY(canSaveAsBMP),
			ENTRY(saveAsBMP),
			ENTRY(canSaveAsPNG),
			ENTRY(saveAsPNG),
			ENTRY(canSaveAsJPG),
			ENTRY(saveAsJPG),
			ENTRY(canSaveAsHDR),
			//			ENTRY(saveAsHDR),
			ENTRY(canSaveAsKTX),
			ENTRY(saveAsKTX),
			ENTRY(canSaveAsDDS),
			ENTRY(saveAsDDS),
#endif
			{nullptr, nullptr}  /* sentinel */
	};

	static const struct luaL_Reg imageLib [] = {
#define ENTRY(name) { #name, &(name) }
			ENTRY(create),
			ENTRY(createNoClear),
			ENTRY(create1D),
			ENTRY(create1DNoClear),
			ENTRY(create1DArray),
			ENTRY(create1DArrayNoClear),

			ENTRY(create2D),
			ENTRY(create2DNoClear),
			ENTRY(create2DArray),
			ENTRY(create2DArrayNoClear),

			ENTRY(create3D),
			ENTRY(create3DNoClear),
			ENTRY(create3DArray),
			ENTRY(create3DArrayNoClear),

			ENTRY(createCubemap),
			ENTRY(createCubemapNoClear),
			ENTRY(createCubemapArray),
			ENTRY(createCubemapArrayNoClear),
#if IKUY_HAVE_LIB_IMAGE_IO == 1
			ENTRY(load),
#endif
			{nullptr, nullptr}  // sentinel
#undef ENTRY
	};

	WRITE_CORE_LOCAL(luaAllocator,allocator);
	luaL_newmetatable(L, MetaName);
	/* metatable.__index = metatable */
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	/* register methods */
	luaL_setfuncs(L, imageObj, 0);

	luaL_register(L, "image", imageLib);
	return 1;
}

#endif