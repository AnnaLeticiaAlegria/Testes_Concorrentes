/*
Module: eventManager.h
Author: Anna Leticia Alegria
Last Modified at: 23/11/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
Description: This module contains the Event Manager header functions. 
The Manager works reading the passed file that contains the desired order of the events. It calls the module 
luaMain.lua to read this file and return if it was possible to read the file or not. The Lua module contains functions
to create the graph of events and to manipulate it, so that the C module only gets the results of the operations.

The documentation explains how the event's file should be done.

After calling the initializeManager function, the user can call the function checkCurrentEvent, passing the desired
event. This event must exist in the event's file. The program calls the Lua function that checks if the desired event 
is possibly the next event, checking the graph's edges from the current event. Besides that, the function checks the
if the thread's id corresponds to the allowed thread's id.
This way, the program can control the event's order and make sure that the threads follow this order.

At the end of the user's program, the finalizeManager function should be called to free all the memory allocated
by this module.
----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* This code assumes that the user's program is in a different folder than this module:
.
|---external directory
| |---user's program folder
| | |---main.c
| | |---eventsFile.txt
| |---EventManager
| | |---grammarParser.lua
| | |---graphManager.lua
| | |---luaMain.lua
| | |---eventManager.c
| | |---eventManager.h
| | |---threadIdManager.lua


Change the luaMainFilePath accordingly and the package.path variable in luaMain.lua module if it is not the case
*/
static char * luaMainFilePath = "../EventManager/luaMain.lua";

/* Time that the program can wait a thread until it considers as a deadlock. Default is 5 seconds */
static int deadLockDetectTime = 5;

/*
----------------------------------------------------------------------------------------------------------------------
Function: initializeManager
Parameters: 
  -> fileName: the name of the file with the events' order
  -> nThreads: user's program's total number of threads
Returns: nothing

Description: This function loads the Lua file called luaMain.lua, located at luaMainFilePath. 
The Lua function converts the file to a graph which corresponds to the possible event's orders.

Input assertions: 
  -> The variable luaMainFilePath must correspond to the correct localization of the Lua file luaMain.lua
  -> The user's program must implement threads using the pthread library
  -> The variable nThreads must correspond to the total number of threads created by the user

Output assertions:
  -> In case the fileName is incorrect, the program will warn and exit with code 0
  -> In case the program fails to create the graph, it will warn the user and exit with code 0
  -> In case of success, the graph will be created and filled correctly. The user can call the checkCurrentEvent 
function now
----------------------------------------------------------------------------------------------------------------------
*/
void initializeManager (char * scriptFileName, char * configFileName);


/*
----------------------------------------------------------------------------------------------------------------------
Function: checkCurrentEvent
Parameters: 
  -> event: name of the event that wants to go next
Returns: nothing

Description: This functions checks if the given event is a possible next event in the order described by the eventFile
passed and if it fullfills the thread's id's condition. For more details about the id's condition, check the 
documentation.

Input assertions: 
  -> The user must have called the function initializeManager before this function (Therefore, it must fullfill all
this functions' input assertions as well)
  -> The given event name must exist in the eventFile passed

Output assertions:
  -> In case it is this event's turn, the function returns so the user program's can continue
  -> In case it is not, the thread keeps waiting in this function until it is it's turn
----------------------------------------------------------------------------------------------------------------------
*/
void checkCurrentEvent (const char * event);


void checkCurrentEventWithId (const char * event, int eventId);


/*
----------------------------------------------------------------------------------------------------------------------
Function: finalizeManager
Parameters: none
Returns: nothing

Description: This function releases all the variables allocated by the function initializeManager.

Input assertions: 
  -> The user must have called the function initializeManager before this function (Therefore, it must fullfill all
this functions' input assertions as well)
  -> This function must be called at the end of the user's program.

Output assertions:
  -> The allocated spaces will be released
----------------------------------------------------------------------------------------------------------------------
*/
void finalizeManager (void);