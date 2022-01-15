/*
 * What matters here is the size of the display, which should reflect
 * the size of 20, which is written with an _ character. Weird, but
 * legal.
 */

module underscore_in_size;
initial
	$display ( "%d %d %d %d", 2_0'b0, 2_0'd0, 2_0'o0, 2_0'h0 );
endmodule
