module main;

   wire [15:0] out;
   reg [16:0]  in;
   reg [15:0]  mask;

   mask dut (.\output (out), .\input (in[15:0]), .mask(mask));

   wire [15:0] out_ref = in[15:0] & mask;
   initial begin
      for (in = 0 ; in[16] == 0 ; in = in+1) begin
	 mask = $random;
	 #1 if (out !== out_ref) begin
	    $display("FAILED: in=%b, out=%b, mask=%b, out_ref=%b", in, out, mask, out_ref);
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule // main
