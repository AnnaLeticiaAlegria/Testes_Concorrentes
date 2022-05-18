#!/bin/bash

gcc -Wall -o queue main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

# testing article`s case
./queue 3 4 1 ../Queue/EventsFiles/eventOrder1.txt >&1 | tee ./Tests/Logs/EventsVersion/test1.log

# testing dequeue waits -----explicar que executa o evento dentro do if, mostrando que ta esperando mesmo (seria equivalente a testar se es'ta errado)
./queue 1 1 1 ../Queue/EventsFiles/eventOrder2.txt ../Queue/EventsFiles/configurationFile1.txt >&1 | tee ./Tests/Logs/EventsVersion/test2.log

# testing enqueue waits
./queue 1 2 1 ../Queue/EventsFiles/eventOrder3.txt ../Queue/EventsFiles/configurationFile1.txt >&1 | tee ./Tests/Logs/EventsVersion/test3.log

# # testing infinite script
# ./queue 4 10 10 ../Queue/EventsFiles/eventOrder4.txt >&1 | tee ./Tests/Logs/EventsVersion/test4.log

# testing infinite script
./queue 4 10 10 ../Queue/EventsFiles/eventOrder4_5.txt >&1 | tee ./Tests/Logs/EventsVersion/test4_5.log


# testing why Continues is important
./queue 2 2 1 ../Queue/EventsFiles/eventOrder6.txt ../Queue/EventsFiles/configurationFile2.txt >&1 | tee ./Tests/Logs/EventsVersion/test6.log