#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int main (void) {
  lua_State* LState = luaL_newstate();
  luaL_openlibs(LState);

  if (luaL_loadfile(LState, "../Tests/mainFile.lua")) {
    printf("Error opening lua file\n");
    exit(0);
  }

  /* Priming run */
  if (lua_pcall(LState, 0, 0, 0)) {
    printf("Error in priming run --> %s\n", lua_tostring(LState, -1));
    exit(0);
  }

  /* Put the function's name and its parameters in the stack */
  lua_getglobal(LState, "boo");

  /* Call the function on the stack, giving 1 parameter and expecting 1 value to be returned */
  if (lua_pcall(LState, 0, 0, 0)) {
    printf("Error during boo call --> %s\n", lua_tostring(LState, -1));
    exit(0);
  }

  lua_close(LState);

  return 0;
}