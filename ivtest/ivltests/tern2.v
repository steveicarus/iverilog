/*
 * This program doesn't do anything, and shouldn't be run. This is
 * only to check that the null target can see the ternary operator.
 */

module main2( );

   reg	      sel;
   reg [13:0] out;
   reg [13:0] a, b;

// This assign works OK
//   assign out[13:0]	= ( sel ? a[13:0] : b[13:0] );

   always @(
	    sel or
	    a or
	    b
	   )
     begin
	out[13:0] = ( sel ? a[13:0] : b[13:0] );
     end
endmodule
