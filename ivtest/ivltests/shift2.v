module main;

   reg [7:0] xu;
   reg signed [7:0] xs;

   initial begin
      xu = 8'b1100_0000;
      xs = 8'b1100_0000;

      xu = xu >>> 3;
      xs = xs >>> 3;

      if (xu !== 8'b0001_1000) begin
	 $display("FAILED -- Unsigned >>> failed. xu=%b", xu);
	 $finish;
      end

      if (xs !== 8'b1111_1000) begin
	 $display("FAILED -- Signed >>> failed. xs=%b", xs);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
