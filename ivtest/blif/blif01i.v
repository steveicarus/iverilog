
module ivtest
  (
   input [3:0]	x,
   input [3:0]	y,
   output [3:0]	z
   );

   assign z = x ^ y;

endmodule // ivtest

