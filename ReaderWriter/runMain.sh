#!/bin/bash

printf "\nRunning Test 1 --> ReaderWriter with 2 readers, 1 writer and statesFile_1.txt....\n\n"

sleep 1

gcc -Wall -o readerWriter main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./readerWriter 2 1 ../ReaderWriter/StatesFiles/statesFile_1.txt >&1 | tee ./Tests/Logs/test1.log

sleep 1

printf "\nRunning Test 2 --> ReaderWriter with 2 readers, 1 writer and statesFile_2.txt....\n\n"

sleep 1

gcc -Wall -o readerWriter main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./readerWriter 2 1 ../ReaderWriter/StatesFiles/statesFile_2.txt >&1 | tee ./Tests/Logs/test2.log

sleep 1

printf "\nRunning Test 3 --> ReaderWriter with 2 readers, 2 writers and statesFile_3.txt....\n\n"

sleep 1

gcc -Wall -o readerWriter main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./readerWriter 2 2 ../ReaderWriter/StatesFiles/statesFile_3.txt >&1 | tee ./Tests/Logs/test3.log

sleep 1

printf "\nRunning Test 4 --> ReaderWriter with 2 readers, 2 writers and statesFile_4.txt....\n\n"

sleep 1

gcc -Wall -o readerWriter main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./readerWriter 2 2 ../ReaderWriter/StatesFiles/statesFile_4.txt >&1 | tee ./Tests/Logs/test4.log

sleep 1

printf "\nRunning Test 5 --> ReaderWriter with 1 reader, 1 writer and statesFile_5.txt....\n\n"

sleep 1

gcc -Wall -o readerWriter main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./readerWriter 1 1 ../ReaderWriter/StatesFiles/statesFile_5.txt >&1 | tee ./Tests/Logs/test5.log

sleep 1

printf "\nRunning Test 6 --> ReaderWriter with 1 reader, 1 writer and statesFile_6.txt....\n\n"

sleep 1

gcc -Wall -o readerWriter main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

./readerWriter 1 1 ../ReaderWriter/StatesFiles/statesFile_6.txt >&1 | tee ./Tests/Logs/test6.log

sleep 1