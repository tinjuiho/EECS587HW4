#!/bin/bash

g++ main.cpp -fopenmp -o main.o -O3
# g++ main.cpp -o main.o
sbatch submit.job