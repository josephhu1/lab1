## UID: 105955662

## Pipe Up

This program will simulate the pipe operator in shells and execute programs as new processes
with a pipe between two subsequent processes.

## Building

To build the program, use:
$ make

## Running

Here is an example run of the program:
$ ./pipe date cat wc
1   7   32

The program should produce the same output as:
$ date | cat | wc

## Cleaning up

To clean up all binary files, use:
$ make clean
