#!/bin/bash

gcc -Wall -o firstExampleWithSemaphore main_with_semaphore.c ../EventManager/EventManager.c -I/$LUA_CDIR -llua5.3

printf "\nRunning firstExample without semaphores passing eventOrderFile_1.txt....\n\n"

sleep 2

./firstExampleWithSemaphore 2 ../FirstExample/EventsFiles/eventOrderFile_1.txt >&1 | tee ./Tests/Logs/EventsVersion/test1_with_semaphore.log

sleep 2

printf "\n\nRunning firstExample without semaphores passing eventOrderFile_2.txt....\n\n"

sleep 2

./firstExampleWithSemaphore 2 ../FirstExample/EventsFiles/eventOrderFile_2.txt >&1 | tee ./Tests/Logs/EventsVersion/test2_with_semaphore.log