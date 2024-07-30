#include "parser.h"
#include "jsmn.h"
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 4096

static void parse_json(lua_State *L, const char *json, jsmntok_t *tokens, int *index);

int json_to_lua_table(lua_State *L, const char *json)
{
  jsmn_parser parser;
  jsmntok_t tokens[MAX_TOKENS];
  jsmn_init(&parser);

  int token_count = jsmn_parse(&parser, json, strlen(json), tokens, MAX_TOKENS);

  if (token_count < 0) {
    switch (token_count) {
    case JSMN_ERROR_NOMEM:
      luaL_error(L, "JSON string is too big");
      break;
    case JSMN_ERROR_INVAL:
      luaL_error(L, "JSON string is corrupted");
      break;
    case JSMN_ERROR_PART:
      luaL_error(L, "JSON string is too short");
      break;
    default:
      luaL_error(L, "Failed to parse JSON");
    }
    return 0;
  }

  int index = 0;
  parse_json(L, json, tokens, &index);

  return 1; // 1 onto stack
}

static void parse_json(lua_State *L, const char *json, jsmntok_t *tokens, int *index)
{
  jsmntok_t *token = &tokens[*index];
  (*index)++; // next token

  switch (token->type) {
  case JSMN_PRIMITIVE: {
    char *value = strndup(json + token->start, token->end - token->start);
    if (strchr("0123456789-.", value[0]) != NULL) { // number?
      lua_pushinteger(L, atoll(value));             // TODO: Opacity is a float
    }
    else if (strchr(value, 't') != NULL) { // true
      lua_pushboolean(L, 1);
    }
    else if (strchr(value, 'f') != NULL) { // false
      lua_pushboolean(L, 0);
    }
    free(value);
    break;
  }
  case JSMN_STRING: {
    lua_pushlstring(L, json + token->start, token->end - token->start);
    break;
  }
  case JSMN_OBJECT: {
    lua_newtable(L);
    int i;
    for (i = 0; i < token->size; i++) {
      parse_json(L, json, tokens, index); // key
      parse_json(L, json, tokens, index); // value
      lua_settable(L, -3);
    }
    break;
  }
  case JSMN_ARRAY: {
    lua_newtable(L);
    int i;
    for (i = 0; i < token->size; i++) {
      lua_pushinteger(L, i + 1);
      parse_json(L, json, tokens, index);
      lua_settable(L, -3);
    }
    break;
  }
  default:
    luaL_error(L, "Unexpected JSON token");
  }
}
