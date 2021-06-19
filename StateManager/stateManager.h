/*
Module: stateManager.h
Author: Anna Leticia Alegria
Last Modified at: 17/06/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
Description: This module contains the State Manager header functions. 
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* 
----------------------------------------------------------------------------------------------------------------------
Global variable's declaration
----------------------------------------------------------------------------------------------------------------------
*/

/* This code assumes that the user's program is in a different folder than this module:
.
|---external directory
| |---user's program folder
| | |---main.c
| | |---statesFile.txt
| |---StateManager
| | |---stateManager.c
| | |---stateManager.h
| | |---readStates.lua

Change the readStatesFilePath accordingly if it is not the case
*/
static char * readStatesFilePath = "../StateManager/readStates.lua";

/* Time that the program can wait a thread until it considers as a deadlock. Default is 5 seconds */
static int deadLockDetectTime = 5;

/*
----------------------------------------------------------------------------------------------------------------------
Function: initializeManager
Parameters: 
  -> fileName: the name of the file with the states' order
  -> nThreads: user's program's total number of threads
Returns: nothing

Description: This function loads the Lua file called readStates.lua, located at readStatesFilePath. 
This function allocates and fills the arrays needed to the manager's work.

Input assertions: 
  -> The variable readStatesFilePath must correspond to the correct localization of the Lua file readStates.lua
  -> The user's program must implement threads using the pthread library
  -> The variable nThreads must correspond to the total number of threads created by the user
  -> The id's in the statesFile file must be valid (See documentation)

Output assertions:
  -> In case the fileName is incorrect, the program will warn and exit with code 0
  -> In case the program fails to allocate the array's space, it will warn the user and exit with code 0
  -> In case of success, the arrays will be created and filled correctly. The user can call the checkState function now
----------------------------------------------------------------------------------------------------------------------
*/
void initializeManager (char * fileName, int nThreads);


/*
----------------------------------------------------------------------------------------------------------------------
Function: checkState
Parameters: 
  -> state: name of the state that wants to go next
Returns: nothing

Description: This functions checks if the given state is the next state in the order described by the stateFile passed
and if it fullfills the id's condition. For more details about the id's condition, check the documentation.

Input assertions: 
  -> The user must have called the function initializeManager before this function (Therefore, it must fullfill all
this functions' input assertions as well)
  -> The given state name must exist in the stateFile passed

Output assertions:
  -> In case it is this state's turn, the function returns so the user program's can continue
  -> In case it is not, the thread keeps waiting in this function until it is it's turn
----------------------------------------------------------------------------------------------------------------------
*/
void checkState (const char * state);


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