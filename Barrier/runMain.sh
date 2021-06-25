#!/bin/bash

gcc -Wall -o barrier main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

printf "\nTest 1 --> Running Barrier v1 with 3 threads, 2 steps and statesFile_1.txt....\n\n"

sleep 2

./barrier 1 3 2 ../Barrier/StatesFiles/statesFile_1.txt >&1 | tee ./Tests/Logs/test1.log

sleep 2

printf "\nTest 2 --> Running Barrier v2 with 5 threads, 1 step and statesFile_2.txt....\n\n"

sleep 2

./barrier 2 5 1 ../Barrier/StatesFiles/statesFile_2.txt >&1 | tee ./Tests/Logs/test2.log

sleep 2

printf "\nTest 3 --> Running Barrier v3 with 5 threads, 1 step and statesFile_2.txt....\n\n"

sleep 2

./barrier 3 5 1 ../Barrier/StatesFiles/statesFile_2.txt >&1 | tee ./Tests/Logs/test3.log

printf "\nTest 4 --> Running Barrier v3 with 5 threads, 1 step and statesFile_3.txt....\n\n"

sleep 2

./barrier 3 5 1 ../Barrier/StatesFiles/statesFile_3.txt >&1 | tee ./Tests/Logs/test4.log