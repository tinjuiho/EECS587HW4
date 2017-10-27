#!/bin/bash

g++ main.cpp -fopenmp -o main.o
# g++ main.cpp -o main.o
sbatch submit.job