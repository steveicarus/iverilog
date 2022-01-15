////
//// The following was written to illustrate a bug in iverilog.
//// In particular, this little lovely produces a 202 MB vvp file.
////

module ExplodedArrays1;

   reg [7:0] data [0:25600-1];

   integer   idx;
   initial begin
      for (idx = 0 ;  idx < 25600 ;  idx = idx+1)
        data[idx] = idx[7:0];

      for (idx = 0 ;  idx < 256 ;  idx = idx+ 1) begin
	 if (data[idx] !== idx) begin
	    $display("FAILED -- data[%d] = %d (%h)", idx, data[idx], data[idx]);
	    $finish;
	 end
      end

      $display("PASSED");
   end // initial begin

endmodule
