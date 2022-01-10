#!/bin/bash

gcc -Wall -o producerConsumer main.c ../EventManager/eventManager.c -I/$LUA_CDIR -llua5.3

printf "\nTest 1 --> Running ProducerConsumerPassingTheBaton with buffer with 1 position, 1 producer, 2 consumers and 1 item passing eventOrderFile_1.txt....\n\n"

sleep 2

./producerConsumer 1 1 2 1 ../ProducerConsumerPassingTheBaton/EventsFiles/eventOrderFile_1.txt >&1 | tee ./Tests/Logs/EventsFiles/test1.log

sleep 2


printf "\nTest 2 --> Running ProducerConsumerPassingTheBaton with buffer with 1 position, 1 producer, 2 consumers and 1 item passing statesFile_2.txt....\n\n"

sleep 2

./producerConsumer 1 1 2 1 ../ProducerConsumerPassingTheBaton/EventsFiles/statesFile_2.txt >&1 | tee ./Tests/Logs/EventsFiles/test2.log

sleep 2

printf "\nTest 3 --> Running ProducerConsumerPassingTheBaton with buffer with 1 position, 2 producers, 2 consumers and 2 items passing statesFile_3.txt....\n\n"

sleep 2

./producerConsumer 1 2 2 2 ../ProducerConsumerPassingTheBaton/EventsFiles/statesFile_3.txt >&1 | tee ./Tests/Logs/EventsFiles/test3.log

sleep 2


# 1 -> 1 produtor e 2 consumidores
# ./producerConsumer 2 1 1 4 ../ProducerConsumerPassingTheBaton/New_StatesFile/statesFile4.txt

./producerConsumer 1 1 2 1 ../ProducerConsumerPassingTheBaton/Tests/test.txt