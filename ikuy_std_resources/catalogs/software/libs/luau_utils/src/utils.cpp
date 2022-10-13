#include "lua.h"
#include "lualib.h"
/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
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
const char* luaL_getTableStringAt(lua_State *L, int index, const char * key) {
	lua_pushstring(L, key);
	lua_gettable (L, index);
	return luaL_checkstring(L, -1);
}

int luaL_getTableIntegerAt(lua_State *L, int index, const char * key) {
	lua_pushstring(L, key);
	lua_gettable (L, index);
	return luaL_checkinteger(L, -1);
}

bool luaL_getTableBooleanAt(lua_State *L, int index, const char * key) {
	lua_pushstring(L, key);
	lua_gettable (L, index);
	return luaL_checkboolean(L, -1);
}