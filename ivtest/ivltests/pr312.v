module main;

   reg [1:0] x;
   reg [2:0] y;

   initial begin
      x = 1;
      y = {1'b0, x << 1};

      if (y !== 3'b010) begin
	 $display("FAILED -- y (%b) != 3'b010", y);
	 $finish;
      end

      y = {1'b0, x << 2};
      if (y !== 3'b000) begin
	 $display("FAILED -- y (%b) != 3'b000", y);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
