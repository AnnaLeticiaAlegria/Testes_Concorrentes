#!/bin/bash

gcc -Wall -o producerConsumer main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

./producerConsumer $1 $2 ../ProducerConsumer/EventsFiles/eventOrder$3.txt