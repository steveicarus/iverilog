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
   defparam main.xx.sub[0].U.number = 0;
   defparam main.xx.sub[1].U.number = 1;
   defparam main.xx.sub[2].U.number = 2;
   defparam main.xx.sub[3].U.number = 3;
   defparam main.xx.sub[4].U.number = 4;

   initial begin
      clk = 0;
      #1 clk = 1;
      while (clk != 0)
	 #1 clk = clk << 1;
      $finish(0);
   end

endmodule // main

module dut #(parameter wid = 1) (input [wid-1:0] clk);
   genvar i;
   for (i = 0 ; i < wid ; i = i+1) begin : sub
      target U (.clk(clk[i]));
   end
endmodule //

module target(input wire clk);

   parameter number = 999;
   always @(posedge clk)
     $display("%m: number=%0d", number);

endmodule // target
