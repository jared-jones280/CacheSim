# CacheSim
Coding

Write a simple cache simulator. Write your program in C or C++ and call it cache.c (for C) or cache.cc (for C++). The program should work by the following specifications:

Command-Line Parameters

There should be four command line parameters in this order:

nk: the capacity of the cache in kilobytes (an int)

assoc: the associativity of the cache (an int)

blocksize: the size of a single cache block in bytes (an int)

repl: the replacement policy (a char); 'l' means LRU, 'r' means random.

Input

Read traces from the standard input. Each line on the standard input will be a lowercase 'r' (for read) or 'w' (for write) followed by a space and then a 64-bit hexadecimal number giving the address of the memory access. For example, a snippet of a trace file looks like this:

r 56ecd8

r 47f639

r 7ff0001ff

w 47f63e

r 4817ef

r 7d5ab8

Output

The output should be a single line of six numbers separated by spaces. These six numbers are:

The total number of misses,

The percentage of misses (i.e. total misses divided by total accesses),

The total number of read misses,

The percentage of read misses (i.e. total read misses divided by total read accesses),

The total number of write misses,

The percentage of write misses (i.e. total write misses divided by total write accesses).

