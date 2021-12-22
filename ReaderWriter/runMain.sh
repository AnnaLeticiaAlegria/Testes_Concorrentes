#!/bin/bash

gcc -Wall -o readerWriter main.c ../EventManager/eventManager.c -I/$LUA_CDIR -llua5.3

printf "\nRunning Test 1 --> ReaderWriter with 1 reader, 1 writer and eventOrderFile_1.txt....\n\n"

sleep 1

./readerWriter 1 1 ../ReaderWriter/EventsFiles/eventOrderFile_1.txt >&1 | tee ./Tests/Logs/EventsVersion/test1.log

sleep 1

printf "\nRunning Test 2 --> ReaderWriter with 1 reader, 1 writer and eventOrderFile_2.txt....\n\n"

sleep 1

./readerWriter 1 1 ../ReaderWriter/EventsFiles/eventOrderFile_2.txt >&1 | tee ./Tests/Logs/EventsVersion/test2.log

sleep 1

printf "\nRunning Test 3 --> ReaderWriter with 2 readers, 1 writer and eventOrderFile_3.txt....\n\n"

sleep 1

./readerWriter 2 1 ../ReaderWriter/EventsFiles/eventOrderFile_3.txt >&1 | tee ./Tests/Logs/EventsVersion/test3.log

sleep 1

printf "\nRunning Test 4 --> ReaderWriter with 2 readers, 1 writer and stateeeventOrderFile_4ventOrderFile_1sFile_4.txt....\n\n"

sleep 1

./readerWriter 2 1 ../ReaderWriter/EventsFiles/eventOrderFile_4.txt >&1 | tee ./Tests/Logs/EventsVersion/test4.log

sleep 1

printf "\nRunning Test 5 --> ReaderWriter with 1 reader, 1 writer and eventOrderFile_5.txt....\n\n"

sleep 1

./readerWriter 1 1 ../ReaderWriter/EventsFiles/eventOrderFile_5.txt >&1 | tee ./Tests/Logs/EventsVersion/test5.log

sleep 1

printf "\nRunning Test 6 --> ReaderWriter with 2 readers, 2 writers and eventOrderFile_2.txt....\n\n"

sleep 1

./readerWriter 2 2 ../ReaderWriter/EventsFiles/eventOrderFile_2.txt >&1 | tee ./Tests/Logs/EventsVersion/test6.log

sleep 1