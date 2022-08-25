#!/bin/bash

gcc -Wall -o ../Queue/queue ../Queue/main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

if [ $1 == 1 ]; then
  # testing dequeue waits -----explicar que executa o evento dentro do if, mostrando que ta esperando mesmo (seria equivalente a testar se es'ta errado)
  ../Queue/queue 1 1 1 ../Queue/EventsFiles/eventOrder0.txt ../Queue/EventsFiles/configurationFile1.txt >&1 | tee ../Queue/Tests/Logs/EventsVersion/test0.log
elif [ $1 == 2 ]; then
  # testing article`s case  
  ../Queue/queue 3 4 1 ../Queue/EventsFiles/eventOrder1.txt >&1 | tee ../Queue/Tests/Logs/EventsVersion/test1.log
else
  # testing why Continues is important
  ../Queue/queue 2 2 1 ../Queue/EventsFiles/eventOrder6.txt ../Queue/EventsFiles/configurationFile2.txt >&1 | tee ../Queue/Tests/Logs/EventsVersion/test6.log
fi

# # testing infinite script
# ./queue 4 10 10 ../Queue/EventsFiles/eventOrder4.txt >&1 | tee ./Tests/Logs/EventsVersion/test4.log

# testing infinite script
# ../Queue/queue 4 10 10 ../Queue/EventsFiles/eventOrder4_5.txt >&1 | tee ../Queue/Tests/Logs/EventsVersion/test4_5.log


