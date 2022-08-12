/*
Module: eventManager.c
Author: Anna Leticia Alegria
Last Modified at: 23/11/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
Description: This module contains the Event Manager functions. 
The Manager works reading the passed file that contains the desired order of the events. It calls the module 
luaMain.lua to read this file and create an graph that contains the possible paths an event can lead to. This graph
is operated by the lua functions. This C module calls this functions whenever an event needs to be checked.

The documentation explains how the event's file should be done.

After the event's graph is ready, the user can call the function checkCurrentEvent, passing the desired event. 
This event must exist in the event's file. The program checks if the desired event can be the next on the graph if 
it fullfills the thread's id's conditions.
This way, the program can control the event's order and make sure that the threads follow this order.

At the end of the user's program, the finalizeManager function should be called to free all the memory allocated
by this module.
----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
*/

#include "eventManager.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

/* 
----------------------------------------------------------------------------------------------------------------------
Global variable's declaration
----------------------------------------------------------------------------------------------------------------------
*/

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t conditionLock = PTHREAD_MUTEX_INITIALIZER;

lua_State *LState;

/*
----------------------------------------------------------------------------------------------------------------------
Encapsulated function's declaration
----------------------------------------------------------------------------------------------------------------------
*/

void callConfigFunction (char * configFileName);
void callReadEventsFunction (char * eventsFileName);
int compareEvents (const char * event);
void setLuaPath(lua_State* L, const char* path);
void signalHandler(int signum);

/*
----------------------------------------------------------------------------------------------------------------------
Function: initializeManager
Parameters: 
  -> fileName: the name of the file with the events' order
  -> nThreads: user's program's total number of threads
Returns: nothing

Description: This function loads the Lua file called luaMain.lua, located at luaMainFilePath. It exits in case
this file doesn't exist. Next, the function prepares the stack to call the Lua functions in luaMain.lua.
Since the function readStates requires a string with the path of the file, the program puts this variable in the stack,
after putting the function's name. After that, it call lua_pcall to call the Lua function.
After that, it closes the LuaState.
Lastly, it sets an alarm with time 'deadLockDetectTime' (which is, by default, 5 seconds)
----------------------------------------------------------------------------------------------------------------------
*/
void initializeManager (char * scriptFileName, char * configFileName) {

  /* Set SIGALRM handler */
  signal(SIGALRM,signalHandler);

  /* Open Lua State */
  LState = luaL_newstate();
  luaL_openlibs(LState); 


  // setLuaPath(LState, "/Users/annaleticiaalegria/Documentos/Paralelo/Testes_Concorrentes/StateManager/grammarParser.lua");

  /* Load Lua file */
  if (luaL_loadfile(LState, luaMainFilePath)) {
    printf("Error opening lua file\n");
    exit(0);
  }

  /* Priming run */
  if (lua_pcall(LState, 0, 0, 0)) {
    printf("Error in priming run --> %s\n", lua_tostring(LState, -1));
    exit(0);
  }

  callConfigFunction (configFileName);

  callReadEventsFunction (scriptFileName);

  /* Set alarm of 'deadLockDetectTime' seconds */
  alarm(deadLockDetectTime);
}


/*
----------------------------------------------------------------------------------------------------------------------
Function: checkCurrentEvent
Parameters: 
  -> event: name of the event that wants to go next
Returns: nothing

Description: This functions checks if the given event can be the next event in the event's graph order and if it 
fullfills the id's condition. For more details about the id's condition, check the function compareEvents.
If so, it prints the event and calls the pthread function that sends a message in broadcast to all the waiting 
threads so they can stop waiting and check if its their turn now.
If it isn't this events's turn, the thread keeps waiting until another thread sinalize that it can stop waiting.

There is an alarm with time of 'deadLockDetectTime' seconds declared everytime an event begins it's turn. If this time 
runs out, it means no other event started a turn for the last 'deadLockDetectTime' seconds. This means that every 
thread is waiting for the turn of their event and none of them has been accepted. This indicate that the sequence
in the event File is not a valid sequence for the user's program. So, the handler of the SIGALRM ends the manager
and the user's program.
----------------------------------------------------------------------------------------------------------------------
*/
void checkCurrentEvent (const char * event) {

  while (1) {
    pthread_mutex_lock(&conditionLock); // P(conditionLock)

    /* Check if it's this state's turn */
    if(compareEvents(event)) {
      // printf("%s\n", event);
      fflush (stdout);
      pthread_cond_broadcast(&condition); // Tell to the awaiting threads that they can go on
      pthread_mutex_unlock(&conditionLock); // V(conditionLock)
      alarm(deadLockDetectTime);
      return;
    }

    pthread_cond_wait(&condition, &conditionLock); // Thread keeps waiting until another thread tells to go on

    pthread_mutex_unlock(&conditionLock); // V(conditionLock)
  }
  
}

void checkCurrentEventWithId (const char * event, int eventId) {
  char * eventName, idChar[5];
  int n;

  n = strlen(event);
  eventName = (char*) malloc ((n + 1 + 10) * sizeof(char));
  if (eventName == NULL) {
    printf("Error during eventName alloc\n");
    exit(0);
  }

  strcpy(eventName, event);
  sprintf(idChar, "%d", eventId);
  strcat(eventName, idChar);

  checkCurrentEvent(eventName);
  free(eventName);
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
Function: compareEvents
Parameters: 
  -> event: name of the event that wants to go next
Returns:
  -> 1: if the given event can be the next event
  -> 0: if the given event cannot be the next event. Therefore, it needs to wait its turn

Description: This function calls the Lua function that compares the given event name with all the possible next 
event's name of the current node of the graph to see if it can be the next event to go on. This Lua function also
checks if this thread's id fullfills the thread's id's condition.

The accepted thread's by every event are defined by the user using it's name within brackets. The user must follow
this form:

[term1 assignSignal term2]

In which:
  -> term1: It can be empty. It defines the group of threads in which the thread that is trying to execute this event
  must be part of. (Note: the user must have assigned a thread to this group so this can work, since the group starts
  empty).
  -> assignSignal: It determines what will happen to the thread that executed this event:
    --> If assignSignal = '>>', then the group described in term2 will be overwrited by the thread that executed this
    event.
    --> If assignSignal = '>>+', then the thread that executed this event will be added to the group of threads
    described in term2.
    --> If assignSignal = '>>-', then the thread that executed this event will be subtracted of the group of threads
    described in term2.
----------------------------------------------------------------------------------------------------------------------
*/
int compareEvents (const char * event) {
  int isEventNext;

   /* Put the function's name and its parameters in the stack */
  lua_getglobal(LState, "checkEvent");
  lua_pushlstring(LState, event, strlen(event));
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

/*
----------------------------------------------------------------------------------------------------------------------
Function: signalHandler
Parameters: 
  -> signum: number of the signal that caused this handler to be called
Returns: nothing

Description: This handler is called whenever the SIGALRM reaches the time that it was specified. If another call of
alarm is done, the time will reset (only one alarm can be active at the same time)
This function calls the Lua function "expectedEvent", that returns a table with all the events that could be the next
event after the last event executed. This function then prints the array returned by the Lua function. If there were
no possible next events, this functions prints a message telling everything worked without any problems.
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

  printf("\n\n\nEND");
  if (!code) {
    printf("\n\n\nPossible blocking detected!!\n\nThe input event sequence wasn't accepted by your program. Please note that:\n"); 
    printf("-- If the input sequence was supposed to be invalid, this means that your program might be ok.\n");
    printf("-- If the input sequence was supposed to be valid, this means that your program might have some issues.\n\n");

    printf("Possible Events Expected:\n");
    for (i=0; i< n_nextEvent; i++) {
      printf("\t -> %s\n", nextEventTable[i]);
    }
  }
  else {
    printf("\n\n\nFinished executing the script!!\n\nThe input event sequence was accepted by your program. Please note that:\n");
    printf("-- If the input sequence was supposed to be invalid, this means that your program might have some issues.\n");
    printf("-- If the input sequence was supposed to be valid, this means that your program might be ok.\n\n");
  }
  printf("\n\n\n");

  free(nextEventTable);
  finalizeManager ();
  exit(0);
}

void callConfigFunction (char * configFileName) {
  int isConfig;

  if (configFileName) {
    /* Put the function's name and its parameters in the stack */
    lua_getglobal(LState, "readConfigFile");
    lua_pushlstring(LState, configFileName, strlen(configFileName));  

    /* Call the function on the stack, giving 1 parameter and expecting 1 value to be returned */
    if (lua_pcall(LState, 1, 1, 0)) {
      printf("Error during readConfigFile call --> %s\n", lua_tostring(LState, -1));
      exit(0);
    }

    /* If the configuration file could be read */
    isConfig = lua_tonumber(LState, -1);
    lua_pop(LState,1);

    if (!isConfig) {
      printf("Error in configuration file --> %s\n", lua_tostring(LState, -1));
      exit(0);
    }
  }
}

void callReadEventsFunction (char * eventsFileName) {
  int existsTree;

  /* Put the function's name and its parameters in the stack */
  lua_getglobal(LState, "readEventsFile");
  lua_pushlstring(LState, eventsFileName, strlen(eventsFileName));  

  /* Call the function on the stack, giving 1 parameter and expecting 1 value to be returned */
  if (lua_pcall(LState, 1, 1, 0)) {
    printf("Error during readEventsFile call --> %s\n", lua_tostring(LState, -1));
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
}