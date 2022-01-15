
module main;

   reg [3:0] x, y;
   wire [3:0] z;

   ivtest dut (.\x[3] (x[3]), .\x[2] (x[2]), .\x[1] (x[1]), .\x[0] (x[0]),
	       .\y[3] (y[3]), .\y[2] (y[2]), .\y[1] (y[1]), .\y[0] (y[0]),
	       .\z[3] (z[3]), .\z[2] (z[2]), .\z[1] (z[1]), .\z[0] (z[0]));

   integer   idx;
   initial begin
      for (idx = 0 ; idx[8]==0 ; idx = idx+1) begin
	 x = idx[3:0];
	 y = idx[7:4];
	 #1 /* let devices settle. */ ;
	 if (z !== (x ^ y)) begin
	    $display("FAILED -- x=%b, y=%b, x^y=%b", x, y, z);
	    $finish;
	 end
      end
      $display("PASSED");
   end // initial begin

endmodule // main
