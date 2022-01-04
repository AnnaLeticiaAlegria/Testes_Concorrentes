#!/bin/bash

gcc -Wall -o queue main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

./queue $1 $2 ../Queue/EventsFiles/eventOrder$3.txt