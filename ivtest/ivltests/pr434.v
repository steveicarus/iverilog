/*
 * This tests is based on PR#434
 */

`define VAR1 2
`define VAR2 5
module mctrl( reset0, reset1, reset2, reset3, clk, por);
   output reset0, reset1, reset2, reset3;
   input  clk,por;

   reg [7:0] cnt;

   always @ (posedge por or posedge clk)
     if (por)
       cnt <= 0;
     else
       cnt <= cnt+1;

   assign    reset0 = (cnt == `VAR1);
   assign    reset1 = (cnt == `VAR2);
   assign    reset2 = (cnt == `VAR1 + `VAR2);
   assign    reset3 = (cnt == `VAR1 + `VAR2 + 2);

endmodule

`timescale 1ns/1ps

module test();
   reg clk,por;

   wire reset0, reset1, reset2, reset3;

   mctrl m1(reset0, reset1, reset2, reset3, clk, por);

   initial
     begin
	clk = 0;
	por = 0;
	$monitor($time,, "reset0=%b, reset1=%b, reset2=%b, reset3=%b",
		 reset0, reset1, reset2, reset3);
	#1000 $finish(0);
     end

   always #15 clk = ~clk;


   initial
     begin
	#10 por = 1;
	#10 por = 0;
     end
endmodule // test
