#!/bin/bash

gcc -Wall -o producerConsumer main.c ../EventManager/eventManager.c ../ConcurrencyModule/concurrency.c -I/$LUA_CDIR -llua5.3

printf "\nTesting with 1 producer, 1 consumer and eventOrder1 ......\n\n\n"

./producerConsumer 1 1 1 ../ProducerConsumer/EventsFiles/eventOrder1.txt >&1 | tee ./Tests/Logs/test1.log

sleep 2

printf "\nTesting with 1 producer, 1 consumer and eventOrder2 ......\n\n\n"

./producerConsumer 1 1 1 ../ProducerConsumer/EventsFiles/eventOrder2.txt >&1 | tee ./Tests/Logs/test2.log

printf "\nTesting with 2 producers, 2 consumers and eventOrder3 ......\n\n\n"

./producerConsumer 2 2 1 ../ProducerConsumer/EventsFiles/eventOrder3.txt >&1 | tee ./Tests/Logs/test3.log

printf "\nTesting with 2 producers, 2 consumers and eventOrder4 ......\n\n\n"

./producerConsumer 2 2 1 ../ProducerConsumer/EventsFiles/eventOrder4.txt >&1 | tee ./Tests/Logs/test4.log

printf "\nTesting with 3 producers, 3 consumers and eventOrder5 ......\n\n\n"

./producerConsumer 3 3 1 ../ProducerConsumer/EventsFiles/eventOrder5.txt >&1 | tee ./Tests/Logs/test5.log