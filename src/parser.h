#pragma once
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

int json_to_lua_table(lua_State *L, const char *json);
