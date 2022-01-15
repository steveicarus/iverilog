/*
 * This test verifies vpiPureTransportDelay functionality
 */

`timescale 1 ns / 1 ps
module test;
    reg r;
    initial begin
	$monitor("<monitor> r = ", r);
	#0.1 r = 1'b0;
	#100000 $finish(0);
    end
    always @(r) $display("<display> r = %b @ %0t", r, $time);

endmodule
