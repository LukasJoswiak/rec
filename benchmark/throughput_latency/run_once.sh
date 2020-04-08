#!/bin/bash

# Spawn four servers.
echo "Starting servers..."
./bin/rec server1 1111 > /dev/null 2>&1 &
SERVER_1_PID=$!
sleep 1

./bin/rec server2 1112 > /dev/null 2>&1 &
SERVER_2_PID=$!
sleep 1

./bin/rec server3 1113 > /dev/null 2>&1 &
SERVER_3_PID=$!
sleep 1

./bin/rec server4 1114 > /dev/null 2>&1 &
SERVER_4_PID=$!

echo "Running client..."
sleep 1

./bin/client localhost 1114 $1 $2 > $3

echo "Client workload complete"
echo "Killing servers..."

kill $SERVER_1_PID
kill $SERVER_2_PID
kill $SERVER_3_PID
kill $SERVER_4_PID
echo "Killed servers, exiting"
