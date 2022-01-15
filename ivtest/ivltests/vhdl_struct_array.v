module main;

   reg [16:0] in;
   wire [15:0] out;

   foo_entity dut (.o_high1(out[15:12]), .o_low1(out[11:8]),
		   .o_high0(out[7:4]),   .o_low0(out[3:0]),

		   .i_high1(in[15:12]), .i_low1(in[11:8]),
		   .i_high0(in[7:4]),   .i_low0(in[3:0]));

   initial begin
      for (in = 0 ; in < 256 ; in = in+1) begin
	 #1 if (in !== out[15:0]) begin
	    $display("FAILED -- out=%h, in=%h", out, in);
	    $finish;
	 end
      end
      $display("PASSED");
   end

endmodule // main
