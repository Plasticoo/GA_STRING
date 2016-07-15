#!/bin/bash

scan-build g++ -std=c++11 -Wall -pipe -march=native -O3 main.cpp -o program
