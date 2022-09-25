//
// Created by deano on 9/25/22.
//
#include "core/core.h"
#include "library_defines/library_defines.h"
#include "memory/memory.hpp"
#include "resource_bundle_writer/resource_bundle_writer.hpp"

#if IKUY_HAVE_LIB_LUAU == 1 || IKUY_HAVE_LIB_LUAU_VM
#include "multi_core/core_local.h"
#include "lua.h"
#include "lualib.h"

#if IKUY_HAVE_LIB_IMAGE == 1
#include "image/resource_writer.hpp"
#include "dbg/print.h"
#include "host_os/osvfile.h"

#endif

/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
static void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
	luaL_checkstack(L, nup, "too many upvalues");
	for (; l->name != NULL; l++) {  /* fill the table with given functions */
		int i;
		for (i = 0; i < nup; i++)  /* copy upvalues to the top */
			lua_pushvalue(L, -nup);
		lua_pushcclosure(L, l->func, l->name, nup);  /* closure with those upvalues */
		lua_setfield(L, -(nup + 2), l->name);
	}
	lua_pop(L, nup);  /* remove upvalues */
}

#define LUA_ASSERT(test, state, msg) if(!(test)) { luaL_error((state), (msg)); }
static char const MetaName[] = "ikuy.ResourceBundleWriter";
static CORE_LOCAL(Memory_Allocator * , luaAllocator);

static void rbw_gc (void *rbw_) {
	auto rbw = *(Binny::BundleWriter **)rbw_;
	if (rbw) {
		FREE_CLASS(luaAllocator, BundleWriter, rbw);
	}
}

// create the null image user data return on the lua state
static Binny::BundleWriter const** rbwud_create(lua_State *L) {
	// allocate a pointer and push it onto the stack
	auto ud = (Binny::BundleWriter const**)lua_newuserdatadtor(L, sizeof(Binny::BundleWriter*), &rbw_gc);
	if(ud == nullptr) return nullptr;

	*ud = nullptr;
	luaL_getmetatable(L, MetaName);
	lua_setmetatable(L, -2);
	return ud;
}

static int create(lua_State *L) {
	int bitWidth = luaL_checkinteger(L, 1);
	auto ud = rbwud_create(L);
	*ud = ALLOC_CLASS(luaAllocator, Binny::BundleWriter, bitWidth, luaAllocator);
	if(*ud) {
		auto rbw = (Binny::BundleWriter*) *ud;
		tiny_stl::vector<uint32_t> dependecies(luaAllocator);
#if IKUY_HAVE_LIB_IMAGE == 1
		debug_print("Registering IMG_ writer\n");
		rbw->registerChunk("IMG_"_bundle_id,
	                       0,
	                       dependecies,
	                       ImageChunkWriter,
	                       ImageChunkWriter);
#endif
	}
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int getTextMode(lua_State * L) {
	auto rbw = *(Binny::BundleWriter **)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(rbw, L, "BundleWriter is NIL");
	lua_pushboolean(L, rbw->getLogBinifyText() );
	return 1;
}

static int setTextMode(lua_State * L) {
	auto rbw = *(Binny::BundleWriter **)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(rbw, L, "BundleWriter is NIL");
	bool textMode = luaL_checkboolean(L, 2);
	rbw->setLogBinifyText(textMode);
	return 0;
}

static int addItemToChunk(lua_State * L) {
	auto rbw = *(Binny::BundleWriter **)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(rbw, L, "BundleWriter is NIL");
	const char * resourceType = luaL_checkstring(L, 2);
	const char * resourceName = luaL_checkstring(L, 3);
	auto p = *(void **)lua_touserdata(L, 4);

	LUA_ASSERT( p, L, "Invalid item passed to addItemToChunk");
	LUA_ASSERT( strlen(resourceType) == 4, L, "Resource Type must be 4 characters exactly");
	uint32_t const resourceId = RESOURCE_BUNDLE_ID(resourceType[0], resourceType[1], resourceType[2], resourceType[3] );
	rbw->addItemToChunk( resourceId, tiny_stl::string(resourceName, luaAllocator), p);

	return 0;
}
static int buildToFile(lua_State * L) {
	auto rbw = *(Binny::BundleWriter **)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(rbw, L, "BundleWriter is NIL");
	const char * fileName = luaL_checkstring(L, 2);
	auto file = Os_VFileFromFile(fileName, Os_FM_WriteBinary, luaAllocator);
	bool result = rbw->build(file);
	VFile_Close(file);
	lua_pushboolean(L, result );
	return 1;
}

int LuaBundleWriter_Open(lua_State* L, Memory_Allocator* allocator) {
	static const struct luaL_Reg rbwObj [] = {
#define ENTRY(name) { #name, &(name) }
		ENTRY( getTextMode ),
		ENTRY( setTextMode ),
		ENTRY( addItemToChunk ),
		ENTRY( buildToFile ),
		{nullptr, nullptr}  /* sentinel */
	};

	static const struct luaL_Reg rbwLib [] = {
		ENTRY(create),
		{nullptr, nullptr}  // sentinel
#undef ENTRY
	};

	WRITE_CORE_LOCAL(luaAllocator,allocator);
	luaL_newmetatable(L, MetaName);
	/* metatable.__index = metatable */
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	/* register methods */
	luaL_setfuncs(L, rbwObj, 0);

	luaL_register(L, "bundle_writer", rbwLib);
	return 1;
}

#endif // end LUA