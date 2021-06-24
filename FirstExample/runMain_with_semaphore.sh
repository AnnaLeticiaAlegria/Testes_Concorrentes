#!/bin/bash

printf "\nRunning firstExample without semaphores passing statesFile_1.txt....\n\n"

sleep 2

gcc -Wall -o firstExampleWithSemaphore main_with_semaphore.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./firstExampleWithSemaphore 2 ../FirstExample/StatesFiles/statesFile_1.txt >&1 | tee ./Tests/Logs/test1_with_semaphore.log

sleep 2

printf "\n\nRunning firstExample without semaphores passing statesFile_2.txt....\n\n"

sleep 2

gcc -Wall -o firstExampleWithSemaphore main_with_semaphore.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./firstExampleWithSemaphore 2 ../FirstExample/StatesFiles/statesFile_2.txt >&1 | tee ./Tests/Logs/test2_with_semaphore.log