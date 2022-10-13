#include "core/core.h"
#include "library_defines/library_defines.h"

#if IKUY_HAVE_LIB_LUAU == 1 || IKUY_HAVE_LIB_LUAU_VM == 1
#include "dbg/print.h"
#include "memory/memory.h"
#include "lua.h"
#include "lualib.h"
#include "luau_utils/utils.hpp"
#include "memory/memory.hpp"
#include "dbg/print.h"
#include "multi_core/core_local.h"
#include "gfx2d/base.hpp"
#include "gfx2d/sprite.hpp"

char const Point2dMetaName[] = "ikuy.Gfx2DPoint2d";
char const Size2dMetaName[] = "ikuy.Gfx2DSize2d";
char const Rect2dMetaName[] = "ikuy.Gfx2DRect2d";
char const SpriteDataMetaName[] = "ikuy.Gfx2DSpriteData";

static CORE_LOCAL(Memory_Allocator * , luaAllocator);
#define ENTRY(name) { #name, &(name) }

// create the null image user data return on the lua state
static void releasePoint2d (void *rbw_) {
	auto rbw = *(Gfx2d::Point2d **)rbw_;
	if (rbw) {
		FREE_CLASS(luaAllocator, Point2d, rbw);
	}
}
static Gfx2d::Point2d const** point2d_userdata_create(lua_State *L) {
	auto ud = (Gfx2d::Point2d const**)lua_newuserdatadtor(L, sizeof(Gfx2d::Point2d*), &releasePoint2d);
	if(ud == nullptr) return nullptr;
	luaL_getmetatable(L, Point2dMetaName);
	lua_setmetatable(L, -2);
	return ud;
}
static int createPoint2d(lua_State *L) {
	// allocate a pointer and push it onto the stack
	auto ud = point2d_userdata_create(L);
	if(ud == nullptr){
		lua_pushnil(L);
		lua_pushboolean(L, false);
		return 2;
	}
	*ud = ALLOC_CLASS(luaAllocator, Gfx2d::Point2d);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}
static int Point2dX(lua_State *L) {
	auto point  = *(Gfx2d::Point2d const **) luaL_checkudata(L, 1, Point2dMetaName);
	LUA_ASSERT(point, L, "point2d is NIL");
	lua_pushinteger(L,  point->x);
	return 1;
}

static int Point2dY(lua_State *L) {
	auto point  = *(Gfx2d::Point2d const **) luaL_checkudata(L, 1, Point2dMetaName);
	LUA_ASSERT(point, L, "point2d is NIL");
	lua_pushinteger(L,  point->y);
	return 1;
}

static int Point2dSetX(lua_State *L) {
	auto point  = *(Gfx2d::Point2d **) luaL_checkudata(L, 1, Point2dMetaName);
	LUA_ASSERT(point, L, "point2d is NIL");
	point->x = (int16_t)luaL_checkinteger(L, 2);

	lua_pushinteger(L,  point->x);
	return 1;
}
static int Point2dSetY(lua_State *L) {
	auto point  = *(Gfx2d::Point2d **) luaL_checkudata(L, 1, Point2dMetaName);
	LUA_ASSERT(point, L, "point2d is NIL");
	point->y = (int16_t)luaL_checkinteger(L, 2);

	lua_pushinteger(L,  point->y);
	return 1;
}
static int Point2dSet(lua_State *L) {
	auto point  = *(Gfx2d::Point2d **) luaL_checkudata(L, 1, Point2dMetaName);
	LUA_ASSERT(point, L, "point2d is NIL");
	point->x = (int16_t)luaL_checkinteger(L, 2);
	point->y = (int16_t)luaL_checkinteger(L, 3);
	return 0;
}
static int Point2dCopy(lua_State *L) {
	auto point  = *(Gfx2d::Point2d **) luaL_checkudata(L, 1, Point2dMetaName);
	auto rhs  = *(Gfx2d::Point2d **) luaL_checkudata(L, 2, Point2dMetaName);
	LUA_ASSERT(point, L, "point2d is NIL");
	LUA_ASSERT(rhs, L, "point2d is NIL");
	point->x = rhs->x;
	point->y = rhs->y;
	return 0;
}

// create the null image user data return on the lua state
static void releaseSize2d (void *rbw_) {
	auto rbw = *(Gfx2d::Size2d **)rbw_;
	if (rbw) {
		FREE_CLASS(luaAllocator, Size2d, rbw);
	}
}
static Gfx2d::Size2d const** size2d_userdata_create(lua_State *L) {
	auto ud = (Gfx2d::Size2d const**)lua_newuserdatadtor(L, sizeof(Gfx2d::Point2d*), &releaseSize2d);
	if(ud == nullptr) return nullptr;
	luaL_getmetatable(L, Size2dMetaName);
	lua_setmetatable(L, -2);
	return ud;
}
static int createSize2d(lua_State *L) {
	// allocate a pointer and push it onto the stack
	auto ud = size2d_userdata_create(L);
	if(ud == nullptr){
		lua_pushnil(L);
		lua_pushboolean(L, false);
		return 2;
	}
	*ud = ALLOC_CLASS(luaAllocator, Gfx2d::Size2d);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int Size2dW(lua_State *L) {
	auto size = *(Gfx2d::Size2d const **) luaL_checkudata(L, 1, Size2dMetaName);
	LUA_ASSERT(size, L, "Size2d is NIL");
	lua_pushinteger(L,  size->w);
	return 1;
}

static int Size2dH(lua_State *L) {
	auto size = *(Gfx2d::Size2d const **) luaL_checkudata(L, 1, Size2dMetaName);
	LUA_ASSERT(size, L, "Size2d is NIL");
	lua_pushinteger(L,  size->h);
	return 1;
}

static int Size2dSetW(lua_State *L) {
	auto size  = *(Gfx2d::Size2d **) luaL_checkudata(L, 1, Size2dMetaName);
	LUA_ASSERT(size, L, "Size2d is NIL");
	size->w = (int16_t) luaL_checkinteger(L, 2);

	lua_pushinteger(L,  size->w);
	return 1;
}
static int Size2dSetH(lua_State *L) {
	auto size = *(Gfx2d::Size2d **) luaL_checkudata(L, 1, Size2dMetaName);
	LUA_ASSERT(size, L, "Size2d is NIL");
	size->h = (int16_t) luaL_checkinteger(L, 2);

	lua_pushinteger(L,  size->h);
	return 1;
}

static int Size2dSet(lua_State *L) {
	auto size = *(Gfx2d::Size2d **) luaL_checkudata(L, 1, Size2dMetaName);
	LUA_ASSERT(size, L, "Size2d is NIL");
	size->w = (int16_t) luaL_checkinteger(L, 2);
	size->h = (int16_t) luaL_checkinteger(L, 3);
	return 0;
}
static int Size2dCopy(lua_State *L) {
	auto size  = *(Gfx2d::Size2d **) luaL_checkudata(L, 1, Size2dMetaName);
	auto rhs  = *(Gfx2d::Size2d **) luaL_checkudata(L, 2, Size2dMetaName);
	LUA_ASSERT(size, L, "Size2d is NIL");
	LUA_ASSERT(rhs, L, "Size2d is NIL");
	size->w = rhs->w;
	size->h = rhs->h;
	return 0;
}
static void releaseRect2d (void *rbw_) {
	auto rbw = *(Gfx2d::Rect2d **)rbw_;
	if (rbw) {
		FREE_CLASS(luaAllocator, Rect2d, rbw);
	}
}
static Gfx2d::Rect2d const** rect2d_userdata_create(lua_State *L) {
	auto ud = (Gfx2d::Rect2d const**)lua_newuserdatadtor(L, sizeof(Gfx2d::Rect2d*), &releaseRect2d);
	if(ud == nullptr) return nullptr;
	luaL_getmetatable(L, Rect2dMetaName);
	lua_setmetatable(L, -2);
	return ud;
}
static int createRect2d(lua_State *L) {
	// allocate a pointer and push it onto the stack
	auto ud = rect2d_userdata_create(L);
	if(ud == nullptr){
		lua_pushnil(L);
		lua_pushboolean(L, false);
		return 2;
	}
	*ud = ALLOC_CLASS(luaAllocator, Gfx2d::Rect2d);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int Rect2dOrigin(lua_State *L) {
	auto rect = *(Gfx2d::Rect2d const **) luaL_checkudata(L, 1, Rect2dMetaName);
	createPoint2d(L);
	auto point = (Gfx2d::Point2d **) luaL_checkudata(L, 2,Point2dMetaName);
	if(point != nullptr){
		lua_pop(L,1);
		**point = rect->origin;
	}
	return 1;
}

static int Rect2dSize(lua_State *L) {
	auto rect = *(Gfx2d::Rect2d const **) luaL_checkudata(L, 1, Rect2dMetaName);
	createSize2d(L);
	auto size = (Gfx2d::Size2d **) luaL_checkudata(L, 2,Size2dMetaName);
	if(size != nullptr){
		lua_pop(L,1);
		**size = rect->size;
	}
	return 2;
}

static int Rect2dSetOrigin(lua_State *L) {
	auto rect = *(Gfx2d::Rect2d **) luaL_checkudata(L, 1, Rect2dMetaName);
	auto point = *(Gfx2d::Point2d const **) luaL_checkudata(L, 2,Point2dMetaName);
	rect->origin = *point;
	return 0;
}
static int Rect2dSetSize(lua_State *L) {
	auto rect = *(Gfx2d::Rect2d **) luaL_checkudata(L, 1, Rect2dMetaName);
	auto size = *(Gfx2d::Size2d const **) luaL_checkudata(L, 2,Size2dMetaName);
	rect->size = *size;
	return 0;
}
static void releaseSpriteData(void *rbw_) {
	auto rbw = *(Gfx2d::SpriteData **)rbw_;
	if (rbw) {
		FREE_CLASS(luaAllocator, SpriteData, rbw);
	}
}
static Gfx2d::SpriteData const** spritedata_userdata_create(lua_State *L) {
	auto ud = (Gfx2d::SpriteData const**)lua_newuserdatadtor(L, sizeof(Gfx2d::SpriteData*), &releaseSpriteData);
	if(ud == nullptr) return nullptr;
	luaL_getmetatable(L, SpriteDataMetaName);
	lua_setmetatable(L, -2);
	return ud;
}
static int createSpriteData(lua_State *L) {
	// allocate a pointer and push it onto the stack
	auto ud = spritedata_userdata_create(L);
	if(ud == nullptr){
		lua_pushnil(L);
		lua_pushboolean(L, false);
		return 2;
	}
	*ud = ALLOC_CLASS(luaAllocator, Gfx2d::SpriteData);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

int LuaGfx2d_Open(lua_State* L, Memory_Allocator* allocator) {

	WRITE_CORE_LOCAL(luaAllocator,allocator);

	static const struct luaL_Reg point2dFuncs [] = {
		{"x", &Point2dX},
		{"y", &Point2dY},
		{"setX", &Point2dSetX},
		{"setY", &Point2dSetY},
		{"set", &Point2dSet},
		{"copy", &Point2dCopy},
		{nullptr, nullptr}  /* sentinel */
	};

	// make the structs lua bindings
	luaL_newmetatable(L, Point2dMetaName);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs( L, point2dFuncs, 0);

	static const struct luaL_Reg size2dFuncs [] = {
		{"w", &Size2dW},
		{"h", &Size2dH},
		{"setW", &Size2dSetW},
		{"setH", &Size2dSetH},
		{"set", &Size2dSet},
		{"copy", &Size2dCopy},
		{nullptr, nullptr}  /* sentinel */
	};
	// make the structs lua bindings
	luaL_newmetatable(L, Size2dMetaName);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs( L, size2dFuncs, 0);

	static const struct luaL_Reg rect2dFuncs [] = {
		{"origin", &Rect2dOrigin},
		{"size", &Rect2dSize},
		{"setOrigin", &Rect2dSetOrigin},
		{"setSize", &Rect2dSetSize},
		{nullptr, nullptr}  /* sentinel */
	};
	// make the structs lua bindings
	luaL_newmetatable(L, Rect2dMetaName);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs( L, rect2dFuncs, 0);

	static const struct luaL_Reg spriteDataFuncs [] = {
		{nullptr, nullptr}  /* sentinel */
	};
	// make the structs lua bindings
	luaL_newmetatable(L, SpriteDataMetaName);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs( L, rect2dFuncs, 0);

	// make the gfx2d lua bindings
	static const struct luaL_Reg gfx2dLib [] = {
		ENTRY(createPoint2d),
		ENTRY(createSize2d),
		ENTRY(createRect2d),
		ENTRY(createSpriteData),
		{nullptr, nullptr}  // sentinel
#undef ENTRY
	};

	luaL_register(L, "gfx2d", gfx2dLib);
	return 1;
}

#endif