/*
 * Passing an unpacked array to a function from a continuous assignment is not
 * supported: a .ufunc node is driven by nexuses and an unpacked array is not a
 * net. Check that this is reported rather than asserting.
 */
module top;
   logic [7:0] a [0:3];
   logic [7:0] y;

   function automatic logic [7:0] sum4(input logic [7:0] v [0:3]);
      logic [7:0] r;
      begin
	 r = '0;
	 for (int i = 0; i < 4; i++) r += v[i];
	 return r;
      end
   endfunction

   assign y = sum4(a);

   initial begin
      $display("FAILED -- should not have compiled");
      $finish;
   end
endmodule
