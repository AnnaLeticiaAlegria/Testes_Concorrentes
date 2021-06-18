#include "stateManager.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t conditionLock = PTHREAD_MUTEX_INITIALIZER;

int currentState = 0;
int totalStates = 0;

const char ** statesArray;
int * statesIdArray;
pthread_t * threadIdArray;

void getLuaResults (lua_State *LState);
int compareStates (const char * state, int currentState);

void checkState (const char * state) {

  while (1) {
    pthread_mutex_lock(&conditionLock); //P(conditionLock)

    /* in case there are no states left */
    if (currentState == totalStates) {
      pthread_mutex_unlock(&conditionLock); //V(conditionLock)
      pthread_exit(NULL); //end thread
    }

    if(compareStates(state, currentState)) {
      printf("%s\n", state);
      currentState ++;
      pthread_cond_broadcast(&condition);
      pthread_mutex_unlock(&conditionLock); //V(conditionLock)
      return;
    }

    pthread_cond_wait(&condition, &conditionLock);

    pthread_mutex_unlock(&conditionLock); //V(conditionLock)
  }
  
}

void initializeManager (char * fileName, int nThreads) {
  lua_State *LState;

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
  lua_pushlstring(LState, fileName, strlen(fileName));  
  if (lua_pcall(LState, 1, 3, 0)) {
    printf("Error during readStates call\n");
    exit(0);
  }

  /*popping the results from calling Lua function*/
  getLuaResults (LState);

  lua_close(LState);

  threadIdArray = (pthread_t*) malloc (nThreads * sizeof(pthread_t));
  if (threadIdArray == NULL) {
    printf("Error in threadIdArray malloc\n");
    exit(0);
  }

  for(int i = 0; i<nThreads; i++){
    threadIdArray[i] = 0;
  }
}

void finalizeManager (void) {
  free(threadIdArray);
  free(statesArray);
  free(statesIdArray);
}

void getLuaResults (lua_State *LState) {
  int i = 0;

  /* In readStates.lua, the array size is the last value returned, so it is on the top of the stack */
  totalStates = lua_tonumber(LState, -1);
  lua_pop(LState,1);

  statesArray = (const char **) malloc ((totalStates) * sizeof(const char*));
  if (statesArray == NULL) {
    printf("Error in statesArray malloc\n");
    exit(0);
  }

  statesIdArray = (int *) malloc ((totalStates) * sizeof(int));
  if (statesIdArray == NULL) {
    printf("Error in statesIdArray malloc\n");
    exit(0);
  }

  lua_pushnil(LState); 

  while (lua_next(LState, -2) != 0) {
    /* uses 'key' (at index -2) and 'value' (at index -1) */
    statesArray[i] = lua_tostring(LState, -1);
    lua_pop(LState, 1);
    i++;
  }

  lua_pushnil(LState);
  i = 0;
  while (lua_next(LState, -3) != 0) {
    /* uses 'key' (at index -2) and 'value' (at index -1) */
    statesIdArray[i] = lua_tonumber(LState, -1);
    lua_pop(LState, 1);
    i++;
  }
}

int compareStates (const char * state, int currentState) {
  pthread_t currentThreadId = pthread_self();

  /* First, check the state's name */
  if (!strcmp(state, statesArray[currentState])) {
    /* Second, check if it can be any threadId */
    if (statesIdArray[currentState] != 0) {
      pthread_t targetThreadId = threadIdArray[abs(statesIdArray[currentState]) - 1];

      /* Then, check the thread's id */
      if (statesIdArray[currentState] > 0) {
        if (targetThreadId == 0) {
          threadIdArray[abs(statesIdArray[currentState]) - 1] = currentThreadId;
          return 1;
        }
        return pthread_equal(currentThreadId, targetThreadId);
      }

      if (!pthread_equal(currentThreadId, targetThreadId)) {
        return 1;
      }
    }
    else {
      return 1;
    }
  }

  return 0;
}