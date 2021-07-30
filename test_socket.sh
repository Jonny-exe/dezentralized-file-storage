#!/bin/bash
echo "Compiling...\n"
gcc server.c -o server && gcc client.c -o client

echo "Running: \n"
./server &  PIDIOS=$!
./client &  PIDMIX=$!
wait $PIDIOS
wait $PIDMIX
