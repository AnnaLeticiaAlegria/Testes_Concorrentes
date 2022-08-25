#!/bin/bash

gcc -Wall -o ../DiningPhilosophers/diningPhilosophers ../DiningPhilosophers/main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

if [ $1 == 1 ]; then
  ../DiningPhilosophers/diningPhilosophers 5 ../DiningPhilosophers/EventsFiles/eventOrder1.txt ../DiningPhilosophers/EventsFiles/configFile1.txt >&1 | tee ../DiningPhilosophers//Tests/Logs/test1.log
elif [ $1 == 2 ]; then
  ../DiningPhilosophers/diningPhilosophers 5 ../DiningPhilosophers/EventsFiles/eventOrder2.txt ../DiningPhilosophers/EventsFiles/configFile2.txt >&1 | tee ../DiningPhilosophers//Tests/Logs/test2.log
else
  ../DiningPhilosophers/diningPhilosophers 5 ../DiningPhilosophers/EventsFiles/eventOrder3.txt ../DiningPhilosophers/EventsFiles/configFile2.txt >&1 | tee ../DiningPhilosophers//Tests/Logs/test3.log
fi