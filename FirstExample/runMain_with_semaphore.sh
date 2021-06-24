#!/bin/bash

printf "\nRunning firstExample without semaphores passing statesFile_1.txt....\n\n"

sleep 2

gcc -Wall -o firstExampleWithSemaphore main_with_semaphore.c ../StateManager/stateManager.c -I/LUA_CDIR -llua5.3

./firstExampleWithSemaphore 2 ../FirstExample/StatesFiles/statesFile_1.txt

sleep 2

printf "\n\nRunning firstExample without semaphores passing statesFile_2.txt....\n\n"

sleep 2

gcc -Wall -o firstExampleWithSemaphore main_with_semaphore.c ../StateManager/stateManager.c -I/LUA_CDIR -llua5.3

./firstExampleWithSemaphore 2 ../FirstExample/StatesFiles/statesFile_2.txt