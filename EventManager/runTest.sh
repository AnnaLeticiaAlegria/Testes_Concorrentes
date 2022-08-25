#!/bin/bash

if [ $1 == "FirstExample" ]; then
  ../FirstExample/runMain.sh $2 $3
elif [ $1 == "Barrier" ]; then
  ../Barrier/runBarrier.sh $2 $3
elif [ $1 == "Philosopher" ]; then
  ../DiningPhilosophers/runDiningPhilosophers.sh $2
elif [ $1 == "ProducerConsumer" ]; then
  ../ProducerConsumer/runMain.sh $2
elif [ $1 == "Queue" ]; then
  ../Queue/runQueue.sh $2
elif [ $1 == "ReaderWriter" ]; then
  ../ReaderWriter/runMain.sh $2
else
  echo "Please give a valid problem name to test"
fi