/*
 * This module demonstrates the ability to use a defparam to control
 * the instantation of an instance array, and to also control
 * parameter values within the instance array.
 */

module main;

   localparam wid = 5;
   reg [wid-1:0] clk;

   dut xx (.clk(clk));

   // This defparam sets the desired with of the U instance vector.
   defparam main.xx.wid = wid;
   // These defparams set parameters within U instances.
   defparam main.xx.U[0].number = 0;
   defparam main.xx.U[1].number = 1;
   defparam main.xx.U[2].number = 2;
   defparam main.xx.U[3].number = 3;
   defparam main.xx.U[4].number = 4;

   initial begin
      clk = 0;
      #1 clk = 1;
      while (clk != 0)
	 #1 clk = clk << 1;
      $finish(0);
   end

endmodule // main

module dut #(parameter wid = 1) (input [wid-1:0] clk);
   target U [wid-1:0] (.clk(clk));
endmodule //

module target(input wire clk);

   parameter number = 999;
   always @(posedge clk)
     $display("%m: number=%0d", number);

endmodule // target
