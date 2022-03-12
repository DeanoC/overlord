#include "core/core.h"
#include "dbg/print.h"
#include "memory/memory.h"
#include "lua.h"
#include "lualib.h"
#include "host_os/osvfile.h"
#include "host_os/filesystem.h"
#include "Luau/Compiler.h"
#define BACKWARD_HAS_DW 1
#include "platform/host/backward.hpp"

#include <memory>
#include <string>
#include <optional>

extern int LuaImage_Open(lua_State* L, Memory_Allocator* allocator);


std::optional<std::string> readFile(const std::string& name) {
	size_t size = 0;

	if(!Os_FileExists(name.c_str())) {
		char currentDir[1024];
		memset(currentDir, 0, 1024);
		Os_GetCurrentDir(currentDir, 1024);
		debug_printf("File not found at %s%s\n", currentDir, name.c_str());
		return {};
	}
	void* fileMem = Os_AllFromFile(name.c_str(), true, &size, &Memory_GlobalAllocator);
	if(fileMem == nullptr) {
		debug_printf("File loading failed%s\n",  name.c_str());
		return {};
	};
	std::string str;
	str.resize(size);
	memcpy(str.data(), fileMem, size);
	MFREE(&Memory_GlobalAllocator, fileMem);
	return str;
}

struct GlobalOptions
{
		int optimizationLevel = 1;
		int debugLevel = 1;
} globalOptions;

static Luau::CompileOptions copts()
{
	Luau::CompileOptions result = {};
	result.optimizationLevel = globalOptions.optimizationLevel;
	result.debugLevel = globalOptions.debugLevel;
	result.coverageLevel = 0;

	return result;
}

static int lua_loadstring(lua_State* L)
{
	size_t l = 0;
	const char* s = luaL_checklstring(L, 1, &l);
	const char* chunkname = luaL_optstring(L, 2, s);

	lua_setsafeenv(L, LUA_ENVIRONINDEX, false);

	std::string bytecode = Luau::compile(std::string(s, l), copts());
	if (luau_load(L, chunkname, bytecode.data(), bytecode.size(), 0) == 0)
		return 1;

	lua_pushnil(L);
	lua_insert(L, -2); /* put before error message */
	return 2;          /* return nil plus error message */
}

static int finishrequire(lua_State* L)
{
	if (lua_isstring(L, -1))
		lua_error(L);

	return 1;
}

static int lua_require(lua_State* L)
{
	std::string name = luaL_checkstring(L, 1);
	std::string chunkname = "=" + name;

	luaL_findtable(L, LUA_REGISTRYINDEX, "_MODULES", 1);

	// return the module from the cache
	lua_getfield(L, -1, name.c_str());
	if (!lua_isnil(L, -1))
		return finishrequire(L);
	lua_pop(L, 1);

	std::optional<std::string> source = readFile(name + ".luau");
	if (!source)
	{
		source = readFile(name + ".lua"); // try .lua if .luau doesn't exist
		if (!source)
			luaL_argerrorL(L, 1, ("error loading " + name).c_str()); // if neither .luau nor .lua exist, we have an error
	}

	// module needs to run in a new thread, isolated from the rest
	lua_State* GL = lua_mainthread(L);
	lua_State* ML = lua_newthread(GL);
	lua_xmove(GL, L, 1);

	// new thread needs to have the globals sandboxed
	luaL_sandboxthread(ML);

	// now we can compile & run module on the new thread
	std::string bytecode = Luau::compile(*source, copts());
	if (luau_load(ML, chunkname.c_str(), bytecode.data(), bytecode.size(), 0) == 0)
	{
		int status = lua_resume(ML, L, 0);

		if (status == 0)
		{
			if (lua_gettop(ML) == 0)
				lua_pushstring(ML, "module must return a value");
			else if (!lua_istable(ML, -1) && !lua_isfunction(ML, -1))
				lua_pushstring(ML, "module must return a table or function");
		}
		else if (status == LUA_YIELD)
		{
			lua_pushstring(ML, "module can not yield");
		}
		else if (!lua_isstring(ML, -1))
		{
			lua_pushstring(ML, "unknown error while running module");
		}
	}

	// there's now a return value on top of ML; stack of L is MODULES thread
	lua_xmove(ML, L, 1);
	lua_pushvalue(L, -1);
	lua_setfield(L, -4, name.c_str());

	return finishrequire(L);
}

void setupState(lua_State* L)
{
	luaL_openlibs(L);

	static const luaL_Reg funcs[] = {
			{"loadstring", lua_loadstring},
			{"require", lua_require},
			{NULL, NULL},
	};

	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, funcs);
	lua_pop(L, 1);

	LuaImage_Open(L, &Memory_GlobalAllocator);

	luaL_sandbox(L);
}

std::string runCode(lua_State* L, std::string const& name, std::string const& source)
{
	std::string bytecode = Luau::compile(source, copts());

	if (luau_load(L, (std::string("=") + name).c_str(), bytecode.data(), bytecode.size(), 0) != 0)
	{
		size_t len;
		const char* msg = lua_tolstring(L, -1, &len);

		std::string error(msg, len);
		lua_pop(L, 1);

		return error;
	}

	lua_State* T = lua_newthread(L);

	lua_pushvalue(L, -2);
	lua_remove(L, -3);
	lua_xmove(L, T, 1);

	int status = lua_resume(T, NULL, 0);

	if (status == 0)
	{
		int n = lua_gettop(T);

		if (n)
		{
			luaL_checkstack(T, LUA_MINSTACK, "too many results to print");
			lua_getglobal(T, "_PRETTYPRINT");
			// If _PRETTYPRINT is nil, then use the standard print function instead
			if (lua_isnil(T, -1))
			{
				lua_pop(T, 1);
				lua_getglobal(T, "print");
			}
			lua_insert(T, 1);
			lua_pcall(T, n, 0, 0);
		}
	}
	else
	{
		std::string error;

		if (status == LUA_YIELD)
		{
			error = "thread yielded unexpectedly";
		}
		else if (const char* str = lua_tostring(T, -1))
		{
			error = str;
		}

		error += "\nstack backtrace:\n";
		error += lua_debugtrace(T);

		fprintf(stdout, "%s", error.c_str());
	}

	lua_pop(L, 1);
	return std::string();
}

void Usage(char const* programName) {
	printf("%s: lua_script thats run to process images\n", programName);
}

int main(int argc, char const *argv[]) {
	backward::SignalHandling sh;

	if(argc != 2) {
		Usage(argv[0]);
		return 1;
	}

	std::unique_ptr<lua_State, void (*)(lua_State*)> globalState(luaL_newstate(), lua_close);
	lua_State* L = globalState.get();
	setupState(L);

	auto const source = readFile(argv[1]);
	if(!source) {
		printf("%s: Could not read %s\n", argv[0], argv[1]);
		return 2;
	}
	auto const result = runCode(L, argv[1], source.value());
	if(!result.empty()) {
		printf("%s: %s\n", argv[0], result.c_str());
		return 3;
	}
	return 0;
}