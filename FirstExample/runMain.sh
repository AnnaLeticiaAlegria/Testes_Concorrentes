#!/bin/bash

gcc -Wall -o firstExample main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

printf "\nRunning firstExample without semaphores passing statesFile_1.txt....\n\n"

sleep 2

./firstExample 2 ../FirstExample/StatesFiles/statesFile_1.txt >&1 | tee ./Tests/Logs/test1.log

sleep 2

printf "\n\nRunning firstExample without semaphores passing statesFile_2.txt....\n\n"

sleep 2

./firstExample 2 ../FirstExample/StatesFiles/statesFile_2.txt >&1 | tee ./Tests/Logs/test2.log