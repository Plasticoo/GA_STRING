#!/bin/bash

#scan-build g++ -std=c++11 -Wall -pipe -march=native -O3 main.cpp -o program
scan-build clang++ -std=c++11 -Wall -pipe -march=native -ggdb3 main.cpp -o program
#g++ -std=c++11 -pg -Wall -ggdb main.cpp -o program
