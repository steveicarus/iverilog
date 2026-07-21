# IVL_UVM Interface Generator

Welcome to IVL_UVM project - https://github.com/svenka3/ivl_uvm

## Description
This utility contains free, GNU licensed contribution to generate SystemVerilog interface for a given DUT.

To run a demo, do:

1. cd demo_dir
2. make

Make sure you have the iverilog & vvp in your path before running make above.

## About this utility:

Given a Verilog DUT, goal is to generate SystemVerilog (SV) Testbench around it. 
First step in creating SV testbench is to code SystemVerilog interface. This
utility automates that interface generation by using VPI routines. Code is inside:
	- ivlog_vpi_src




