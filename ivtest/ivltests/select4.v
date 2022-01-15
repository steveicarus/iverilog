/*
 * part select in continuous assignment.
 */

`timescale 1ns/1ns

module main;
   reg [3:0] src;
   wire foo = src[3:1] == 3'b101;

   integer   idx;
   initial
     begin
	for (idx = 0 ;  idx < 16 ;  idx = idx+1) begin
	   src = idx;
	   #1 if (foo !== (src[3:1] == 3'b101)) begin
	      $display("FAILED -- src=%b, foo=%b", src, foo);
	      $finish;
	   end
	end
	$display("PASSED");
     end // initial begin
endmodule
