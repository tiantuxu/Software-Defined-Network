#!/bin/bash

g++ -std=c++11 controller.cpp dijkstra.cpp -o controller.bin
g++ -std=c++11 switch.cpp -o switch.bin
