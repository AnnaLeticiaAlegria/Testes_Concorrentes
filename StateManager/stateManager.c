/*
Module: stateManager.c
Author: Anna Leticia Alegria
Last Modified at: 17/06/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
Description: This module contains the State Manager functions. 
The Manager works reading the passed file that contains the desired order of the events. It calls the module 
readStates.lua to read this file and return an array of the desired events (statesArray) and an array of the states id
(statesIdArray), in which each entry corresponds to the state on the same position in statesArray. This is done in
the initalizeManager function.

The documentation explains how the state's file should be done.

After the statesArray and the statesIdArray are ready, the user can call the function checkState, passing the desired
state/event. This state must exist in the state's file. The program checks if the desired event is the next on the
statesArray and if it's id corresponds to the next state's id.
This way, the program can control the event's order and make sure that the threads follow this order.

At the end of the user's program, the finalizeManager function should be called to free all the memory allocated
by this module.
----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
*/

#include "stateManager.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <signal.h>
#include <unistd.h>

/* 
----------------------------------------------------------------------------------------------------------------------
Global variable's declaration
----------------------------------------------------------------------------------------------------------------------
*/

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t conditionLock = PTHREAD_MUTEX_INITIALIZER;

int currentState = 0; // Position of the current state in the statesArray and in the statesIdArray
int totalStates = 0;  // Total state's sequence length

const char ** statesArray; // Array with the names of each event of the sequence. It's length is equal to totalStates 
int * statesIdArray;       // Array with the ids of each event of the sequence. It's length is equal to totalStates 
pthread_t * threadIdArray; // Array with the thread's id of the user's program. It's length is equal to the number of threads

lua_State *LState;


/*
----------------------------------------------------------------------------------------------------------------------
Encapsulated function's declaration
----------------------------------------------------------------------------------------------------------------------
*/

void getLuaResults (lua_State *LState);
int compareStates (const char * state, int currentState);
void setLuaPath(lua_State* L, const char* path);
void signalHandler(int signum);

/*
----------------------------------------------------------------------------------------------------------------------
Function: initializeManager
Parameters: 
  -> fileName: the name of the file with the states' order
  -> nThreads: user's program's total number of threads
Returns: nothing

Description: This function loads the Lua file called readStates.lua, located at readStatesFilePath. It exits in case
this file doesn't exist. Next, the function prepares the stack to call the Lua functions in readStates.lua.
Since the function readStates requires a string with the path of the file, the program puts this variable in the stack,
after putting the function's name. After that, it call lua_pcall to call the Lua function.
Then, call this module's function getLuaResults. This function obtains the returns of the Lua function and places at
the global variables of this module.
After closing the LuaState, this function allocates the threadIdArray with length of nThreads and inserts 0 at each
position.
Lastly, it sets an alarm with time 'deadLockDetectTime' (which is, by default, 5 seconds)
----------------------------------------------------------------------------------------------------------------------
*/
void initializeManager (char * fileName, int nThreads) {
  int existsTree;

  /* Set SIGALRM handler */
  signal(SIGALRM,signalHandler);

  /* Open Lua State */
  LState = luaL_newstate();
  luaL_openlibs(LState); 


  // setLuaPath(LState, "/Users/annaleticiaalegria/Documentos/Paralelo/Testes_Concorrentes/StateManager/grammarParser.lua");

  /* Load Lua file */
  if (luaL_loadfile(LState, readStatesFilePath)) {
    printf("Error opening lua file\n");
    exit(0);
  }

  /* Priming run */
  if (lua_pcall(LState, 0, 0, 0)) {
    printf("Error in priming run --> %s\n", lua_tostring(LState, -1));
    exit(0);
  }

  /* Put the function's name and its parameters in the stack */
  lua_getglobal(LState, "readStatesFile");
  lua_pushlstring(LState, fileName, strlen(fileName));  

  /* Call the function on the stack, giving 1 parameter and expecting 1 value to be returned */
  if (lua_pcall(LState, 1, 1, 0)) {
    printf("Error during readStatesFile call --> %s\n", lua_tostring(LState, -1));
    exit(0);
  }

  /* Pop the results from calling Lua function */
  existsTree = lua_tonumber(LState, -1);
  lua_pop(LState,1);

  if(!existsTree) {
    printf("Error during event's graph creation\n");
    lua_close(LState);
    exit(0);
  }

  /* Set alarm of 'deadLockDetectTime' seconds */
  alarm(deadLockDetectTime);
}


/*
----------------------------------------------------------------------------------------------------------------------
Function: checkState
Parameters: 
  -> state: name of the state that wants to go next
Returns: nothing

Description: This functions checks if the given state is the next state in the statesArray order and if it fullfills 
the id's condition. For more details about the id's condition, check the function compareStates.
If so, it prints the state, updates the current state and calls the pthread function that sends a message in broadcast
to all the waiting threads so they can stop waiting and check if its their turn now.
If it isn't this state's turn, the thread keeps waiting until another thread sinalize that it can stop waiting.

There is an alarm with time of 'deadLockDetectTime' seconds declared everytime a state begins it's turn. If this time 
runs out, it means no other state started a turn for the last 'deadLockDetectTime' seconds. This means that every 
thread is waiting for the turn of their state and none of them has been accepted. This indicate that the sequence
in the state File is not a valid sequence for the user's program. So, the handler of the SIGALRM ends the manager
and the user's program.
----------------------------------------------------------------------------------------------------------------------
*/
void checkState (const char * state) {

  while (1) {
    pthread_mutex_lock(&conditionLock); // P(conditionLock)

    /* In case there are no states left */
    // if (currentState == totalStates) {
    //   pthread_mutex_unlock(&conditionLock); // V(conditionLock)
    //   pthread_exit(NULL); // End thread
    // }

    /* Check if it's this state's turn */
    if(compareStates(state, currentState)) {
      printf("%s\n", state);
      currentState ++;
      pthread_cond_broadcast(&condition); // Tell to the awaiting threads that they can go on
      pthread_mutex_unlock(&conditionLock); // V(conditionLock)
      alarm(deadLockDetectTime);
      return;
    }

    pthread_cond_wait(&condition, &conditionLock); // Thread keeps waiting until another thread tells to go on

    pthread_mutex_unlock(&conditionLock); // V(conditionLock)
  }
  
}


/*
----------------------------------------------------------------------------------------------------------------------
Function: finalizeManager
Parameters: none
Returns: nothing

Description: This function releases all the variables allocated by the function initializeManager. It must be called
at the end of the user's program.
----------------------------------------------------------------------------------------------------------------------
*/
void finalizeManager (void) {
  alarm(0);

  /* Close Lua State */
  lua_close(LState);
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: compareStates
Parameters: 
  -> state: name of the state that wants to go next
  -> currentState: position of the currentState in statesArray
Returns:
  -> 1: if the given state is the next state
  -> 0: if the given state is not the next state. Therefore, it needs to wait its turn

Description: This function first compares the given state name with the current state's name to see if it can be the 
next state to go on. If not, returns 0. If so, continue to check the states id.

The state id is defined by the user on the state's file order. If this state can be performed by any thread, it's id
is equal to 0. If it must be performed by a specific thread, this id contains an integer greater than 0.(*Note 1) 
If it can be performed by any thread but cannot be performed by a specific thread, this id contains an integer lower 
than 0.(*Note 2)

If the thread is the first one passing the name and the positive id's condition, it assignes it's id to the 
threadIdArray at the (id - 1) position.
If the thread passes the name's and the id's condition, the function returns 1.

*Note 1: The user must assign a positive number to this state that is lower or equal to the number of threads. This
number (called id) does not guarantee that this thread is the (id)th thread to be executed, it means that this thread
will be considered the (id)th when it reachs this function. This id is useful when the user wants to guarantee that
differents states/events will be executed by the same thread.

*Note 2: To use this resource, the user must be assigned a thread with the specified id. (For example, if it assigns -1
to a state, it must be any state before assigned to the id 1). Is important to notice that any thread but the specified
can execute this state. Works on changing that are being done.
----------------------------------------------------------------------------------------------------------------------
*/
int compareStates (const char * state, int currentState) {
  int isEventNext;

   /* Put the function's name and its parameters in the stack */
  lua_getglobal(LState, "checkEvent");
  lua_pushlstring(LState, state, strlen(state));
  lua_pushinteger(LState, (int) pthread_self());

  /* Call the function on the stack, giving 1 parameter and expecting 3 values to be returned */
  if (lua_pcall(LState, 2, 1, 0)) {
    printf("Error during checkEvent call--> %s\n", lua_tostring(LState, -1));
    exit(0);
  }

  isEventNext = lua_tonumber(LState, -1);
  lua_pop(LState,1);

  return isEventNext;
}

void setLuaPath( lua_State* L, const char* path )
{
  const char * cur_path;
  char * str_aux;

  lua_getglobal( L, "package" );
  lua_getfield( L, -1, "path" ); // get field "path" from table at top of stack (-1)
  cur_path = lua_tostring( L, -1 ); // grab path string from top of stack

  str_aux = (char*) malloc ((strlen(cur_path) + strlen(path) + 2) * sizeof(char)); // ; + \0
  strcpy(str_aux, cur_path);
  strcat(str_aux, ";");
  strcat(str_aux, path);

  lua_pop( L, 1 ); // get rid of the string on the stack we just pushed on line 5
  lua_pushstring( L, cur_path ); // push the new one
  lua_setfield( L, -2, "path" ); // set the field "path" in table at -2 with value at top of stack
  lua_pop( L, 1 ); // get rid of package table from top of stack

  free(str_aux);
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: signalHandler
Parameters: 
  -> signum: number of the signal that caused this handler to be called
Returns: nothing

Description: This handler is called whenever the SIGALRM reaches the time that it was specified. If another call of
alarm is done, the time will reset (only one alarm can be active at the same time)
----------------------------------------------------------------------------------------------------------------------
*/
void signalHandler(int signum){
  const char ** nextEventTable;
  int n_nextEvent, i=0, code;

  lua_getglobal(LState, "expectedEvent");
  if (lua_pcall(LState, 0, 3, 0)) {
    printf("Error during expectedEvent call--> %s\n", lua_tostring(LState, -1));
    exit(0);
  }

  code = lua_tonumber(LState, -1);
  lua_pop(LState,1);

  lua_pushnil(LState); 

  n_nextEvent = lua_tonumber(LState, -2);
  lua_pop(LState,1);

  /* Allocate nextEventTable */
  nextEventTable = (const char **) malloc ((n_nextEvent) * sizeof(const char*));
  if (nextEventTable == NULL) {
    printf("Error in nextEventTable malloc\n");
    exit(0);
  }

  lua_pushnil(LState); 

  /* Since nextEventTable is an array, the function has to pop everything at this address until it has nothing (which means
  that all the array has been popped) */
  while (lua_next(LState, -3) != 0) {
    /* Since is a Lua table, it has key and value. But in this case, the key is a sequential number in range of 
    (1, #nextEventTable). This function uses 'key' (at index -2) and 'value' (at index -1). The function needs only the
    value, therefore, it only needs what is at index -1 */
    nextEventTable[i] = lua_tostring(LState, -1);
    lua_pop(LState, 1);
    i++;
  }

  lua_pushnil(LState);

  if (!code) {
    printf("\n\n\nDeadLock detected!!\n");

    printf("Possible Events Expected:\n");
    for (i=0; i< n_nextEvent; i++) {
      printf("\t -> %s\n", nextEventTable[i]);
    }
  }
  else {
    printf("\n\n\nFinished executing the script without problems!!\n");
  }
  printf("\n\n\n");

  free(nextEventTable);
  finalizeManager ();
  exit(0);
}