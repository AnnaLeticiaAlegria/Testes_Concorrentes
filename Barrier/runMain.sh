#!/bin/bash

gcc -Wall -o barrier main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

# printf "\nTest 1 --> Running Barrier v1 with 3 threads, 2 steps and eventOrderFile_1.txt....\n\n"

# sleep 2

# ./barrier 1 3 2 ../Barrier/EventsFiles/eventOrderFile_1.txt >&1 | tee ./Tests/Logs/EventsVersion/test1.log

# sleep 2

printf "\nTest 2 --> Running Barrier v2 with 5 threads, 1 step and eventOrderFile_1.txt....\n\n"

sleep 2

./barrier 2 5 1 ../Barrier/EventsFiles/eventOrderFile_1.txt >&1 | tee ./Tests/Logs/EventsVersion/test1.log

sleep 2

printf "\nTest 3 --> Running Barrier v3 with 5 threads, 1 step and eventOrderFile_1.txt....\n\n"

sleep 2

./barrier 3 5 1 ../Barrier/EventsFiles/eventOrderFile_1.txt >&1 | tee ./Tests/Logs/EventsVersion/test2.log

printf "\nTest 4 --> Running Barrier v3 with 5 threads, 1 step and eventOrderFile_2.txt....\n\n"

sleep 2

./barrier 3 5 1 ../Barrier/EventsFiles/eventOrderFile_2.txt >&1 | tee ./Tests/Logs/EventsVersion/test3.log