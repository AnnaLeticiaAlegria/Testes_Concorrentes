#!/bin/bash

gcc -Wall -o diningPhilosophers main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

./diningPhilosophers $1 ../diningPhilosophers/EventsFiles/eventOrder$2.txt ../diningPhilosophers/EventsFiles/configFile$3.txt