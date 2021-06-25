#!/bin/bash

gcc -Wall -o ProducerConsumer main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

printf "\nTest 1 --> Running ProducerConsumerPassingTheBaton with buffer with 1 position, 1 producer, 2 consumers and 1 item passing statesFile_1.txt....\n\n"

sleep 2

./ProducerConsumer 1 1 2 1 ../ProducerConsumerPassingTheBaton/StatesFiles/statesFile_1.txt >&1 | tee ./Tests/Logs/test1.log

sleep 2


printf "\nTest 2 --> Running ProducerConsumerPassingTheBaton with buffer with 1 position, 1 producer, 2 consumers and 1 item passing statesFile_2.txt....\n\n"

sleep 2

./ProducerConsumer 1 1 2 1 ../ProducerConsumerPassingTheBaton/StatesFiles/statesFile_2.txt >&1 | tee ./Tests/Logs/test2.log

sleep 2

printf "\nTest 3 --> Running ProducerConsumerPassingTheBaton with buffer with 1 position, 2 producers, 2 consumers and 2 items passing statesFile_3.txt....\n\n"

sleep 2

./ProducerConsumer 1 2 2 2 ../ProducerConsumerPassingTheBaton/StatesFiles/statesFile_3.txt >&1 | tee ./Tests/Logs/test3.log

sleep 2
