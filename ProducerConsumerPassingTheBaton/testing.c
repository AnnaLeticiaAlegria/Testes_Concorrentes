#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "stateManager.h"

int currentState = 0;
int totalStates = 0;
char ** statesArray;


void getLuaResults (lua_State *LState, int * arraySize, const char ** statesArray, int * statesIdArray) {
  int i = 0;

  /* In readStates.lua, the array size is the last value returned, so it is on the top of the stack */
  *arraySize = lua_tonumber(LState, -1);
  lua_pop(LState,1);

  statesArray = (const char **) malloc ((*arraySize) * sizeof(const char*));
  if (statesArray == NULL) {
    printf("Error in statesArray malloc\n");
    exit(0);
  }

  statesIdArray = (int *) malloc ((*arraySize) * sizeof(int));
  if (statesIdArray == NULL) {
    printf("Error in statesIdArray malloc\n");
    exit(0);
  }
  printf("Tam: %d\n", *arraySize);

  lua_pushnil(LState); 

  while (lua_next(LState, -2) != 0) {
    /* uses 'key' (at index -2) and 'value' (at index -1) */
    statesArray[i] = lua_tostring(LState, -1);
    printf("State: %s\n", statesArray[i]);
    lua_pop(LState, 1);
  }

  lua_pushnil(LState);
  i = 0;
  while (lua_next(LState, -3) != 0) {
    /* uses 'key' (at index -2) and 'value' (at index -1) */
    statesIdArray[i] = lua_tonumber(LState, -1);
    printf("Id: %d\n", statesIdArray[i]);
    lua_pop(LState, 1);
  }
}

int main (int argc, char** argv) {
  lua_State *LState;
  const char ** statesArray = NULL;
  int * statesIdArray = NULL;
  int nStates;

  if (argc != 2) {
    printf("Parâmetros passados incorretamente! (A)\n");
    return 0;
  }

  LState = luaL_newstate();
  luaL_openlibs(LState); 

  if (luaL_loadfile(LState, "readStates.lua")) {
    printf("Error opening lua file\n");
    exit(0);
  }

  if (lua_pcall(LState, 0, 0, 0)) {
    printf("Error in priming run\n");
    exit(0);
  }

  lua_getglobal(LState, "readStates");
  lua_pushlstring(LState, argv[1], strlen(argv[1]));  
  if (lua_pcall(LState, 1, 3, 0)) {
    printf("Error during readStates call\n");
    exit(0);
  }

  /*popping the results from calling Lua function*/
  getLuaResults (LState, &nStates, statesArray, statesIdArray);


  lua_close(LState);
  return 0;
}