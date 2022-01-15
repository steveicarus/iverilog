module main;

   parameter WIDTH = 8;
   parameter ITERATIONS = 1000;

   reg [WIDTH-1:0] src0, src1, ref_dst;
   reg clk;
   wire [WIDTH-1:0] dst;

   test #(.width(WIDTH)) test0 (.dst(dst), .src0(src0), .src1(src1), .clk(clk));

   integer	    idx;
   initial begin
      clk = 0;
      for (idx = 0 ; idx < ITERATIONS ; idx = idx+1) begin
	 src0 = $random;
	 src1 = $random;
	 ref_dst = src0 ^ src1;
	 #1 clk = 1;
	 #1 if (dst !== ref_dst) begin
	    $display("FAILED: src0=%b, src1=%b dst=%b, ref=%b",
		     src0, src1, dst, ref_dst);
	    $finish;
	 end
	 clk = 0;
      end
      $display("PASSED");
   end // initial begin

endmodule // main
