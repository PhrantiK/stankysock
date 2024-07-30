#include "parser.h"
#include "yabai_socket.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_BUF  256
#define MAX_ARGS 20

static int split_argstr(char *argstr, char *argv[MAX_ARGS])
{
  char **ap;
  int argc = 0;

  for (ap = argv; (*ap = strsep(&argstr, " ")) != NULL;) {
    if (**ap != '\0') { // skip extra spaces
      argc++;
      if (++ap >= &argv[MAX_ARGS])
        break;
    }
  }
  *ap = NULL;

  return argc;
}

static void build_argstr(char *buf, size_t bufsize, const char *func_name, const char *props)
{
  snprintf(buf, bufsize, "%s %s --%ss%s%s --%s", "-m", "query", func_name, props ? " " : "",
           props ? props : "", func_name);
}

static void handle_prop_table(lua_State *L, char *buf, size_t bufsize, const char *func_name)
{
  int cur = 0;
  int len = lua_rawlen(L, -1);
  char props[STR_BUF] = {0};

  for (int i = 0; i < len; i++) {
    lua_rawgeti(L, 1, i + 1);
    const char *name = lua_tostring(L, -1);
    cur += snprintf(props + cur, sizeof(props) - cur, "%s%s", i > 0 ? "," : "", name); // trim ,
    lua_pop(L, 1);
  }
  build_argstr(buf, bufsize, func_name, *props ? props : NULL);
}

static int handle_fire_forget(lua_State *L, int argc, char *argv[])
{
  int result = send_cmd(argc, argv);
  lua_pushboolean(L, result == EXIT_SUCCESS);
  return 1;
}

static int handle_get_response(lua_State *L, int argc, char *argv[])
{
  char *response = NULL;
  int result = send_query(argc, (char **)argv, &response);

  if (result == EXIT_SUCCESS && response) {
    json_to_lua_table(L, response);
  }
  else {
    lua_pushnil(L);
    lua_pushstring(L, response ? response : "Failed to message Yabai");
  }
  free(response);

  return (result == EXIT_SUCCESS) ? 1 : 2;
}

static int yabai_message(lua_State *L, bool dontwait)
{
  char *argv[MAX_ARGS] = {NULL};
  char argstr[STR_BUF] = {0};
  int argc = 0;
  const char *func_name = lua_tostring(L, lua_upvalueindex(1));

  if (strcmp(func_name, "query") == 0 || dontwait) {
    const char *input = luaL_checkstring(L, 1);
    snprintf(argstr, sizeof(argstr), "%s", input);
  }
  else if (lua_istable(L, 1)) { // {"app", "id"}
    handle_prop_table(L, argstr, sizeof(argstr), func_name);
  }
  else { // window|display|space()
    build_argstr(argstr, sizeof(argstr), func_name, NULL);
  }

  argc = split_argstr(argstr, argv);

  if (dontwait) {
    return handle_fire_forget(L, argc, argv);
  }
  else {
    return handle_get_response(L, argc, argv);
  }
}

static int lua_yabai_query(lua_State *L)
{
  return yabai_message(L, false); // dontwait = false
}

static int lua_yabai_cmd(lua_State *L)
{
  return yabai_message(L, true); // dontwait = true
}

static const struct luaL_Reg yabai_functions[] = {
    {"query",   lua_yabai_query},
    {"window",  lua_yabai_query},
    {"space",   lua_yabai_query},
    {"display", lua_yabai_query},
    {"cmd",     lua_yabai_cmd  },
    {NULL,      NULL           }
};

int luaopen_stankysock(lua_State *L)
{
  luaL_newlib(L, yabai_functions);

  for (const struct luaL_Reg *l = yabai_functions; l->name; l++) {
    lua_pushstring(L, l->name);
    lua_pushcclosure(L, l->func, 1);
    lua_setfield(L, -2, l->name);
  }
  return 1;
}
