#!/bin/bash

#removed all objects
rm -rf parallel_hashtable
rm -rf parallel_mutex
rm -rf spin_lock
rm -rf parallel_mutex_opt

# Compile the original version
gcc -w -o parallel_hashtable parallel_hashtable.c -lpthread

# Compile the mutex version
gcc -w -o parallel_mutex parallel_mutex.c -lpthread

#Compile the spinlock version
gcc -w -o parallel_spin parallel_spin.c -lpthread

#Compile the mutex opt version
gcc -w -o parallel_mutex_opt parallel_mutex_opt.c -lpthread

# Run the experiments
python run_experiments.py