#!/bin/bash

gcc -Wall -o ../ReaderWriter/readerWriter ../ReaderWriter/main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

if [ $1 == 1 ]; then
  # printf "\nRunning Test 1 --> ReaderWriter with 1 reader, 2 writers and eventOrderFile_1.txt....\n\n"
  ../ReaderWriter/readerWriter 1 2 ../ReaderWriter/EventsFiles/eventOrderFile_1.txt >&1 | tee ../ReaderWriter/Tests/Logs/EventsVersion/test1.log
elif [ $1 == 2 ]; then
  # printf "\nRunning Test 2 --> ReaderWriter with 1 reader, 1 writer and eventOrderFile_2.txt....\n\n"
  ../ReaderWriter/readerWriter 1 1 ../ReaderWriter/EventsFiles/eventOrderFile_2.txt >&1 | tee ../ReaderWriter/Tests/Logs/EventsVersion/test2.log
elif [ $1 == 3 ]; then
# printf "\nRunning Test 3 --> ReaderWriter with 3 readers, 1 writer and eventOrderFile_3.txt....\n\n"
  ../ReaderWriter/readerWriter 3 1 ../ReaderWriter/EventsFiles/eventOrderFile_3.txt >&1 | tee ../ReaderWriter/Tests/Logs/EventsVersion/test3.log
elif [ $1 == 4 ]; then
# printf "\nRunning Test 4 --> ReaderWriter with 3 readers, 3 writers and eventOrderFile_4.txt....\n\n"
  ../ReaderWriter/readerWriter 20 3 ../ReaderWriter/EventsFiles/eventOrderFile_4.txt >&1 | tee ../ReaderWriter/Tests/Logs/EventsVersion/test4.log
else
  ../ReaderWriter/readerWriter 30 30 ../ReaderWriter/EventsFiles/infinite.txt >&1 | tee ../ReaderWriter/Tests/Logs/EventsVersion/infinite.log
fi