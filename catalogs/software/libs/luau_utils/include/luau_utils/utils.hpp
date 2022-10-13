#pragma once

#include "lua.h"

#define LUA_ASSERT(test, state, msg) if(!(test)) { luaL_error((state), (msg)); }

void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);

const char* luaL_getTableStringAt(lua_State *L, int index, const char * key);
int luaL_getTableIntegerAt(lua_State *L, int index, const char * key);
bool luaL_getTableBooleanAt(lua_State *L, int index, const char * key);