module main;

   reg [4:0] src;
   wire      dst;

   test dut(.i(src[3:0]), .o(dst));

   initial begin
      for (src = 0 ; src < 16 ; src = src+1) begin
	 #1 if (dst !== & src[3:0]) begin
	    $display("FAILED: src=%b, dst=%b", src, dst);
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule // main
