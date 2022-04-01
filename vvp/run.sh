#!/bin/bash

set -ex

make
iverilog -g2012 -ohello.vvp hello.sv
g++ -I../install.d/include/iverilog -c test_client.cpp 
g++ -o test_client test_client.o -L../vvp -lvvp
./test_client
