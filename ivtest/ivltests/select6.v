module main;

   reg [7:0] mem [0:1];

   initial begin
      mem[0] = 8'ha5;
      mem[1] = 8'hf0;

      if (mem[0] !== 8'ha5) begin
	 $display("FAILED");
	 $finish;
      end

      if (mem[1] !== 8'hf0) begin
	 $display("FAILED");
	 $finish;
      end

      if (mem[0][4+:4] !== 5'ha) begin
	 $display("FAILED");
	 $finish;
      end

      if (mem[1][4+:4] !== 5'hf) begin
	 $display("FAILED");
	 $finish;
      end

      mem[0][4 +: 4] <= 4'hc;

      #1 if (mem[0] !== 8'hc5) begin
	 $display("FAILED");
	 $finish;
      end

      mem[1][4 +: 4] <= 4'h3;

      #1 if (mem[1] !== 8'h30) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule // main
