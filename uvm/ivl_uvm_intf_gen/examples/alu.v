/**********************************************************************
 *  IVL_UVM example - SystemVerilog Interface generator 
 *
 *
 * Verilog DUT to test the $ivl_uvm_intf_gen
 *
*********************************************************************/

`timescale 1ns / 1ns


module alu (clk, a, b, ci, sum, co);
  input  [7:0] a, b; 
  input ci, clk;
  output [15:0] sum;
  output co;

 `ifdef IVL_UVM_INTF_GEN
  initial
    begin
      #1 $ivl_uvm_intf_gen();
    end
 `endif // IVL_UVM_INTF_GEN
endmodule // alu



