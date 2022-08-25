#!/bin/bash

gcc -Wall -o ../ProducerConsumer/producerConsumer ../ProducerConsumer/main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

if [ $1 == 1 ]; then
  ../ProducerConsumer/producerConsumer 1 1 1 ../ProducerConsumer/EventsFiles/eventOrder1.txt >&1 | tee ../ProducerConsumer/Tests/Logs/test1.log
elif [ $1 == 2 ]; then
  ../ProducerConsumer/producerConsumer 1 1 1 ../ProducerConsumer/EventsFiles/eventOrder2.txt >&1 | tee ../ProducerConsumer/Tests/Logs/test2.log
elif [ $1 == 3 ]; then
  ../ProducerConsumer/producerConsumer 2 2 1 ../ProducerConsumer/EventsFiles/eventOrder3.txt >&1 | tee ../ProducerConsumer/Tests/Logs/test3.log
elif [ $1 == 4 ]; then
  ../ProducerConsumer/producerConsumer 2 2 1 ../ProducerConsumer/EventsFiles/eventOrder4.txt >&1 | tee ../ProducerConsumer/Tests/Logs/test4.log
else
  ../ProducerConsumer/producerConsumer 5 5 3 ../ProducerConsumer/EventsFiles/infinite.txt >&1 | tee ../ProducerConsumer/Tests/Logs/infinite.log
fi