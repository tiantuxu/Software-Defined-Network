#!/bin/bash

#./switch.bin 1 localhost 5000 -f 2 1&
./switch.bin 1 localhost 5000 &
sleep 1
./switch.bin 2 localhost 5000 &
sleep 1
#./switch.bin 3 localhost 5000 -f 6 1 &
./switch.bin 3 localhost 5000 &
./switch.bin 4 localhost 5000 &
sleep 1
#./switch.bin 5 localhost 5000 &
./switch.bin 6 localhost 5000 &


