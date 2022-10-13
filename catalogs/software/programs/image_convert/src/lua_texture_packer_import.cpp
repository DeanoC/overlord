#include "core/core.h"
#include "library_defines/library_defines.h"


#if IKUY_HAVE_LIB_LUAU == 1
#include "dbg/print.h"
#include "memory/memory.h"
#include "lua.h"
#include "lualib.h"
#include "memory/memory.hpp"
#include "data_utils/json.hpp"
#include "host_os/osvfile.h"
#include "host_os/filesystem.hpp"
#include "multi_core/core_local.h"
#include "core/compile_time_hash.hpp"
#include "image/image.h"
#include "luau_utils/utils.hpp"
#include "gfx2d/sprite.hpp"

static char const MetaName[] = "ikuy.TexturePackerImport";
static CORE_LOCAL(Memory_Allocator * , luaAllocator);

struct Point2D {
	float x, y;
};
struct Size2D {
	float w, h;
};

struct Rect2D {
	Point2D p;
	Size2D s;
};

struct Sprite {
	Rect2D frame;
	bool rotated;
	bool trimmed;
	Rect2D spriteSourceSize;
	Size2D sourceSize;
	Point2D pivot;

	std::string name;
	std::string handlerName;
	uint32_t spriteSheetIndex;
};

struct SpriteSheet {
	std::string path;
	std::string format;
	Image_ImageHeader const * image;
};

struct Sprites {
	std::vector<SpriteSheet> spriteSheet;
	std::vector<Sprite> sprites;
};

static void DecodeRect2D( json_object_element_s const *rectField, Rect2D * outRect ) {
	while(rectField != nullptr) {
		char c = rectField->name->string[0];
		assert( rectField->value->type == json_type_number );
		auto num = (struct json_number_s *) rectField->value->payload;
		// TODO I need a string to float in the utils library...
		switch(c) {
			case 'x': outRect->p.x = atof(num->number); break;
			case 'y': outRect->p.y = atof(num->number); break;
			case 'w': outRect->s.w = atof(num->number); break;
			case 'h': outRect->s.h = atof(num->number); break;
			case 0: break;
			default: break;
		}
		rectField = rectField->next;
	}
}

static void DecodeSize2D( json_object_element_s const *sizeField, Size2D * outSize ) {
	while(sizeField != nullptr) {
		char c = sizeField->name->string[0];
		assert( sizeField->value->type == json_type_number );
		auto num = (struct json_number_s *) sizeField->value->payload;
		// TODO I need a string to float in the utils library...
		switch(c) {
			case 'w': outSize->w = atof(num->number); break;
			case 'h': outSize->h = atof(num->number); break;
			case 0: break;
			default: break;
		}
		sizeField = sizeField->next;
	}
}

static void DecodePoint2D( json_object_element_s const *pointField, Point2D * outPoint ) {
	while(pointField != nullptr) {
		char c = pointField->name->string[0];
		assert( pointField->value->type == json_type_number );
		auto num = (struct json_number_s *) pointField->value->payload;
		// TODO I need a string to float in the utils library...
		switch(c) {
			case 'x': outPoint->x = atof(num->number); break;
			case 'y': outPoint->y = atof(num->number); break;
			case 0: break;
			default: break;
		}
		pointField = pointField->next;
	}
}
static void DecodeField( json_object_element_s const *spriteField, Sprite * outSprite) {
	switch(Core::RuntimeHash( strlen(spriteField->name->string), spriteField->name->string )) {
		case "spriteSourceSize"_hash:
		{
			assert( spriteField->value->type == json_type_object );
			auto rect = (struct json_object_s*)spriteField->value->payload;
			DecodeRect2D(rect->start, &outSprite->spriteSourceSize);
			break;
		}
		case "frame"_hash: {
			assert( spriteField->value->type == json_type_object );
			auto rect = (struct json_object_s*)spriteField->value->payload;
			DecodeRect2D(rect->start, &outSprite->frame);
			break;
		}
		case "rotated"_hash: {
			assert( spriteField->value->type == json_type_false ||  spriteField->value->type == json_type_true );
			outSprite->rotated = spriteField->value->type == json_type_true;
			break;
		}
		case "trimmed"_hash:{
			assert( spriteField->value->type == json_type_false ||  spriteField->value->type == json_type_true );
			outSprite->trimmed = spriteField->value->type == json_type_true;
			break;
		}
		case "sourceSize"_hash: {
			assert( spriteField->value->type == json_type_object );
			auto rect = (struct json_object_s*)spriteField->value->payload;
			DecodeSize2D(rect->start, &outSprite->sourceSize);
			break;
		}
		case "pivot"_hash: {
			assert( spriteField->value->type == json_type_object );
			auto rect = (struct json_object_s*)spriteField->value->payload;
			DecodePoint2D(rect->start, &outSprite->pivot);
			break;
		}

		default:
			break;
	}
}

static void DecodeSprite( json_object_element_s const *sprite, Sprites * outSprites ) {
	assert( sprite->value->type == json_type_object );
	auto spriteFields = (struct json_object_s*)sprite->value->payload;
	auto spriteField = spriteFields->start;
	Sprite spriteData {
		.name = sprite->name->string,
		.spriteSheetIndex = 0, // currently only 1 sprite sheet supported
	};

	while(spriteField != nullptr) {
		DecodeField( spriteField, &spriteData );

		spriteField = spriteField->next;
	}
	outSprites->sprites.push_back(spriteData);
}


static void sprites_gc (void *rbw_) {
	auto rbw = *(Sprites **)rbw_;
	if (rbw) {
		FREE_CLASS(luaAllocator, Sprites, rbw);
	}
}

// create the null image user data return on the lua state
static Sprites const** sprites_create(lua_State *L) {
	// allocate a pointer and push it onto the stack
	auto ud = (Sprites const**)lua_newuserdatadtor(L, sizeof(Sprites*), &sprites_gc);
	if(ud == nullptr) return nullptr;

	*ud = nullptr;
	luaL_getmetatable(L, MetaName);
	lua_setmetatable(L, -2);
	return ud;
}

static void* mallocThunk(void * user, size_t size) {
	Memory_Allocator * allocator = (Memory_Allocator *) user;
	return MALLOC(allocator, size);
}

static int load(lua_State *L) {
	const char * path = luaL_checkstring(L, 1);
	const char * fileName = luaL_checkstring(L, 2);

	auto const pathS = std::string(path);
	auto const originalDir = Os::FileSystem::GetCurrentDir();
	auto const singlePageName = std::string(fileName) + ".json";

	Sprites * sprites = ALLOC_CLASS(luaAllocator, Sprites);

	if(!Os::FileSystem::DirExists(pathS)) {
		debug_printf("TexturePackerImport: Directory %s does not exist\n", (Os::FileSystem::GetCurrentDir() + pathS).c_str());
		goto loadExit;
	}
	Os::FileSystem::SetCurrentDir(pathS);

	// check if single page or multi or its doesn't exist
	if(Os::FileSystem::FileExists(singlePageName)) {

		size_t jsonSize {};
		void * jsonTxt = (void *) Os_AllFromFile(singlePageName.c_str(), true, &jsonSize, luaAllocator);
		json_parse_result_s result{};
		json_value_s * jsonRoot = json_parse_ex(jsonTxt, jsonSize, json_parse_flags_default, mallocThunk, luaAllocator, &result);
		assert(jsonRoot->type == json_type_object);
		auto rootObject = (struct json_object_s*)jsonRoot->payload;
		assert(rootObject->length == 2);
		// 2 object
		// 1. frames with the actual sprite data
		// 2. metadata that we can extract the sprite sheet
		auto frameObject = rootObject->start;
		auto metaObject = frameObject->next;

		if(utf8cmp(frameObject->name->string, "frames") == 0) {
			struct json_value_s* spriteObjectsVal = frameObject->value;
			assert(spriteObjectsVal->type == json_type_object);
			auto spriteObjects = (struct json_object_s*)spriteObjectsVal->payload;
			auto sprite = spriteObjects->start;
			while(sprite != nullptr) {
				DecodeSprite( sprite, sprites );
				sprite = sprite->next;
			}
		}
		if(utf8cmp(metaObject->name->string, "meta") == 0) {
			struct json_value_s* metaObjectsVal = metaObject->value;
			assert(metaObjectsVal->type == json_type_object);
			auto metaFields = (struct json_object_s*)metaObjectsVal->payload;
			auto field = metaFields->start;
			SpriteSheet sheet;
			while(field != nullptr) {
				switch(Core::RuntimeHash( strlen(field->name->string), field->name->string )) {
					case "app"_hash:
					case "version"_hash:
						break;
					case "image"_hash: {
						assert( field->value->type == json_type_string );
						auto string = (json_string_s *) field->value->payload;
						sheet.path = pathS + "/" + string->string;
						break;
					}
					case "format"_hash: {
						assert( field->value->type == json_type_string );
						auto string = (json_string_s *) field->value->payload;
						sheet.format = string->string;
						break;
					}
					case "size"_hash:
					case "scale"_hash:
					case "smartupdate"_hash:
					default:
						break;
				}
				field = field->next;
			}
			sprites->spriteSheet.push_back(sheet);
		}

		MFREE(luaAllocator, jsonTxt);
		MFREE(luaAllocator, jsonRoot);

	} else {
		// TODO multi
		debug_printf("TexturePackerImport: Sprite %s.json does not exist\n", fileName);
		goto loadExit;
	}

loadExit:
	Os::FileSystem::SetCurrentDir(originalDir);

	auto ud = sprites_create( L );
	*ud = sprites;
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int spriteSheets(lua_State *L) {
	auto sprites  = *(Sprites const **) luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(sprites, L, "sprites is NIL");
	lua_pushinteger(L, sprites->spriteSheet.size());
	return 1;
}

static int sprites(lua_State *L) {
	auto sprites  = *(Sprites const **) luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(sprites, L, "sprites is NIL");
	lua_pushinteger(L, sprites->sprites.size());
	return 1;
}

static int spriteSheet(lua_State *L) {
	auto sprites  = *(Sprites const **) luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(sprites, L, "sprites is NIL");
	int64_t index = luaL_checkinteger(L, 2);
	LUA_ASSERT(index < sprites->spriteSheet.size(), L, "Sprite Sheet Index too high");

	lua_pushstring(L, sprites->spriteSheet[index].path.c_str());
	lua_pushstring(L, sprites->spriteSheet[index].format.c_str());
	return 2;
}

static int sprite(lua_State *L) {
	auto sprites  = *(Sprites const **) luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(sprites, L, "sprites is NIL");
	int64_t index = luaL_checkinteger(L, 2);
	LUA_ASSERT(index < sprites->sprites.size(), L, "Sprite Index too high");

	lua_newtable(L);
	const int t = lua_gettop(L);

	lua_pushinteger(L, (int)index); lua_setfield(L, t, "spriteIndex");
	lua_pushinteger(L, sprites->sprites[index].frame.p.x); lua_setfield(L, t, "frameX");
	lua_pushinteger(L, sprites->sprites[index].frame.p.y); lua_setfield(L, t, "frameY");
	lua_pushinteger(L, sprites->sprites[index].frame.s.w); lua_setfield(L, t, "frameW");
	lua_pushinteger(L, sprites->sprites[index].frame.s.h);lua_setfield(L, t, "frameH");

	lua_pushboolean(L, sprites->sprites[index].rotated); lua_setfield(L, t, "rotated");
	lua_pushboolean(L, sprites->sprites[index].trimmed); lua_setfield(L, t, "trimmed");

	lua_pushinteger(L, sprites->sprites[index].spriteSourceSize.p.x);  lua_setfield(L, t, "spriteSourceSizeX");
	lua_pushinteger(L, sprites->sprites[index].spriteSourceSize.p.y); lua_setfield(L, t, "spriteSourceSizeY");
	lua_pushinteger(L, sprites->sprites[index].spriteSourceSize.s.w); lua_setfield(L, t, "spriteSourceSizeW");
	lua_pushinteger(L, sprites->sprites[index].spriteSourceSize.s.h);lua_setfield(L, t, "spriteSourceSizeH");
	lua_pushinteger(L, sprites->sprites[index].sourceSize.w); lua_setfield(L, t, "sourceSizeW");
	lua_pushinteger(L, sprites->sprites[index].sourceSize.h);lua_setfield(L, t, "sourceSizeH");
	lua_pushinteger(L, sprites->sprites[index].pivot.x); lua_setfield(L, t, "pivotX");
	lua_pushinteger(L, sprites->sprites[index].pivot.y); lua_setfield(L, t, "pivotY");

	lua_pushstring(L, sprites->sprites[index].name.c_str()); lua_setfield(L, t, "name");
	lua_pushstring(L, sprites->sprites[index].handlerName.c_str()); lua_setfield(L, t, "handlerName");
	lua_pushinteger(L, sprites->sprites[index].spriteSheetIndex); lua_setfield(L, t, "spriteSheetIndex");

	return 1;
}

static int setSpriteDataFrom(lua_State *L) {
	auto sprites  = *(Sprites const **) luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(sprites, L, "sprites is NIL");

	auto spriteData  = *(Gfx2d::SpriteData **) luaL_checkudata(L, 2, "ikuy.Gfx2DSpriteData");
	LUA_ASSERT(spriteData, L, "spriteData is NIL");
	LUA_ASSERT( lua_istable(L, 3), L, "argument must be a table")

	auto spriteIndex = luaL_getTableIntegerAt(L, 3, "spriteIndex");
	auto sprite = sprites->sprites[spriteIndex];
	spriteData->image = sprites->spriteSheet[sprite.spriteSheetIndex].image;
	spriteData->handlerName = luaL_getTableStringAt(L, 3, "handlerName");
	spriteData->spriteSheetCoord.x = luaL_getTableIntegerAt(L, 3, "frameX");
	spriteData->spriteSheetCoord.y = luaL_getTableIntegerAt(L, 3, "frameY");
	spriteData->spriteDimension.w = luaL_getTableIntegerAt(L, 3, "frameW");
	spriteData->spriteDimension.h = luaL_getTableIntegerAt(L, 3, "frameH");
	spriteData->spritePivotPoint.x = luaL_getTableIntegerAt(L, 3, "pivotX");
	spriteData->spritePivotPoint.y = luaL_getTableIntegerAt(L, 3, "pivotY");
	return 1;
}

static int setSpriteSheetImage(lua_State *L) {
	auto sprites  = *(Sprites **) luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(sprites, L, "sprites is NIL");
	int64_t index = luaL_checkinteger(L, 2);
	LUA_ASSERT(index < sprites->spriteSheet.size(), L, "Sprite Sheet Index too high");
	auto image  = *(Image_ImageHeader const **) luaL_checkudata(L, 3, "ikuy.Image");

	sprites->spriteSheet[index].image = image;
	return 1;
}

int LuaTexturePackerImport_Open(lua_State* L, Memory_Allocator* allocator) {
	static const struct luaL_Reg rbwObj [] = {
#define ENTRY(name) { #name, &(name) }
		ENTRY(spriteSheets),
		ENTRY(spriteSheet),
		ENTRY(setSpriteSheetImage),
		ENTRY(sprites),
		ENTRY(sprite),
		ENTRY(setSpriteDataFrom),

		{nullptr, nullptr}  /* sentinel */
	};

	static const struct luaL_Reg rbwLib [] = {
		ENTRY(load),
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

	luaL_register(L, "texture_packer_import", rbwLib);
	return 1;
}

#endif // end LUA