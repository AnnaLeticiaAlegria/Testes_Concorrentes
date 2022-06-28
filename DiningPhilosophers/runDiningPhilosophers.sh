#!/bin/bash

gcc -Wall -o diningPhilosophers main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

./diningPhilosophers 5 ../diningPhilosophers/EventsFiles/eventOrder1.txt ../diningPhilosophers/EventsFiles/configFile1.txt >&1 | tee ./Tests/Logs/test1.log

sleep 2

./diningPhilosophers 5 ../diningPhilosophers/EventsFiles/eventOrder2.txt ../diningPhilosophers/EventsFiles/configFile2.txt >&1 | tee ./Tests/Logs/test2.log

sleep 2

./diningPhilosophers 5 ../diningPhilosophers/EventsFiles/eventOrder3.txt ../diningPhilosophers/EventsFiles/configFile2.txt >&1 | tee ./Tests/Logs/test3.log