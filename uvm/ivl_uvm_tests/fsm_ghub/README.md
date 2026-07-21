# How to add IVL_UVM to an existing TB

## FSM example from:
  https://github.com/shreyshakti/Activity---Simulation-based-Verification-

Used as a simple example to show how IVL_UVM can add value to simple testbenches. It is not intended as demo of full UVM capabilities.

## To add IVL_UVM to an existing TB, follow below steps:

1. Original TB is saved in orig_tb.v for reference, it is a simple linear TB, kindly review
2. Import ivl_uvm_pkg --> line: 5 in fsm_ghub_tb.sv
3. Added a simple monutor code using UVM Messaging feature, lines: 51-53 in fsm_ghub_tb.sv

## To run:

Use the Makefile provided:

  - make orig --> runs Original TB
  - make --> runs with IVL_UVM TB

## Details on IVL_UVM Integration

  - First step is to enable SV parsing in Icarus, use ** -g2012 ** flag
  - Add IVL_UVM files to your command line, done in a config file * ivl_uvm_cmds.cfg *

  Sample commands:

  - iverilog -g2012 -f ivl_uvm_cmds.cfg -o ivl_uvm.vvp
  - vvp  ivl_uvm.vvp


## Sample output:
``` log
VCD info: dumpfile fsm_tb.vcd opened for output.
UVM_INFO fsm_ghub_tb.sv(52) @ 0.000 ns [IVL_GO2UVM] reset: 1 A: 0
UVM_INFO /Users/srini/proj/IVL_UVM/Git_ivl_uvm/ivl_uvm/ivl_uvm_src/ivl_uvm_clp.svh(510) @ 1.000 ns [IVL_GO2UVM] CLP
UVM_INFO @ 1.000 ns [TIMOUTSET] '+UVM_TIMEOUT=1' Global Timeout: 1000000.000 ns
UVM_INFO fsm_ghub_tb.sv(52) @ 20.000 ns [IVL_GO2UVM] reset: 0 A: 0
UVM_INFO fsm_ghub_tb.sv(52) @ 40.000 ns [IVL_GO2UVM] reset: 0 A: 1
UVM_INFO fsm_ghub_tb.sv(52) @ 140.000 ns [IVL_GO2UVM] reset: 0 A: 0
UVM_INFO fsm_ghub_tb.sv(52) @ 240.000 ns [IVL_GO2UVM] reset: 0 A: 1
UVM_INFO fsm_ghub_tb.sv(52) @ 260.000 ns [IVL_GO2UVM] reset: 1 A: 1
fsm_ghub_tb.sv:48: $finish called at 280 (1ns)

--- UVM Report Summary ---

** Report counts by severity
UVM_INFO : 9
UVM_WARNING : 0
UVM_ERROR : 0
UVM_FATAL : 0

*** Congratulations! Test PASSED with NO UVM_ERRORs ***

UVM_INFO /Users/srini/proj/IVL_UVM/Git_ivl_uvm/ivl_uvm/ivl_uvm_src/ivl_uvm_msg.svh(74) @ 280.000 ns [IVL_GO2UVM] Thanks for using IVL_UVM Package

``` 

