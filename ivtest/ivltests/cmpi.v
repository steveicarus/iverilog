
module main;

   reg [7:0] val;

   initial begin
      val = 120;

      if (8'd5 < val) begin
	 $display("OK");
      end else begin
	 $display("FAILED");
	 $finish;
      end

      if (8'd5 <= val) begin
	 $display("OK");
      end else begin
	 $display("FAILED");
	 $finish;
      end

      if (8'd121 > val) begin
	 $display("OK");
      end else begin
	 $display("FAILED");
	 $finish;
      end

      if (8'd121 >= val) begin
	 $display("OK");
      end else begin
	 $display("FAILED");
	 $finish;
      end

      if (val > 8'd5) begin
	 $display("OK");
      end else begin
	 $display("FAILED");
	 $finish;
      end

      if (val >= 8'd5) begin
	 $display("OK");
      end else begin
	 $display("FAILED");
	 $finish;
      end

      if (val < 8'd121) begin
	 $display("OK");
      end else begin
	 $display("FAILED");
	 $finish;
      end

      if (val <= 8'd121) begin
	 $display("OK");
      end else begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
