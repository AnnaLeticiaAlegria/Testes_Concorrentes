#!/bin/bash

printf "\nRunning ReaderWriter with 2 readers, 1 writer and statesFile_1.txt....\n\n"

sleep 1

gcc -Wall -o readerWriter main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./readerWriter 2 1 ../ReaderWriter/StatesFiles/statesFile_1.txt >&1 | tee ./Tests/Logs/test1.log

sleep 1

printf "\nRunning ReaderWriter with 2 readers, 1 writer and statesFile_2.txt....\n\n"

sleep 1

gcc -Wall -o readerWriter main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./readerWriter 2 1 ../ReaderWriter/StatesFiles/statesFile_2.txt >&1 | tee ./Tests/Logs/test2.log

sleep 1

printf "\nRunning ReaderWriter with 2 readers, 2 writers and statesFile_3.txt....\n\n"

sleep 1

gcc -Wall -o readerWriter main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./readerWriter 2 2 ../ReaderWriter/StatesFiles/statesFile_3.txt >&1 | tee ./Tests/Logs/test3.log

sleep 1

printf "\nRunning ReaderWriter with 2 readers, 2 writers and statesFile_4.txt....\n\n"

sleep 1

gcc -Wall -o readerWriter main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./readerWriter 2 2 ../ReaderWriter/StatesFiles/statesFile_4.txt >&1 | tee ./Tests/Logs/test4.log

sleep 1