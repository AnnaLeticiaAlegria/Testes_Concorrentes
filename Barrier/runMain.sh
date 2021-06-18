#!/bin/bash

printf "\nRunning Barrier statesFile_1.txt....\n\n"

sleep 2

gcc -Wall -o barrier main.c ../StateManager/stateManager.c -I/Users/annaleticiaalegria/lua-5.3.5/src -llua5.3

./barrier 5 ../Barrier/StatesFiles/statesFile_1.txt