`ifdef __ICARUS__
  `define SUPPORT_REAL_NETS_IN_IVTEST
`endif

module top;
   reg pass = 1'b1;
   integer scale = 2, offset = 1;
   real    rin;

`ifdef SUPPORT_REAL_NETS_IN_IVTEST
   wire    real ress = scale * rin;
   wire    real reso = rin + offset;
`endif

   initial begin
`ifdef SUPPORT_REAL_NETS_IN_IVTEST
      #1 if (ress != 0.0 || reso != 1.0) begin
	 $display("FAILED: initial value, expected 0.0/1.0, got %f/%f", ress, reso);
	 pass = 1'b0;
      end
      rin = 2.0;
      #1 if (ress != 4.0 || reso != 3.0) begin
	 $display("FAILED: rin=%f, scale=%f, expected 2.0/2.0, got %f/%f", rin, scale, ress, reso);
	 pass = 1'b0;
      end
`endif

      if (pass)
	$display("PASSED");
   end
endmodule
