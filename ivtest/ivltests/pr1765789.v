// pr1765789

module main;

   reg [32:0] addr = {1'b1, 32'h0040_0000 + 32'h8};

   initial begin
      #1 ;
      if (addr !== 33'h1_0040_0008) begin
	 $display("FAILED -- addr = %h", addr);
	 $finish;
      end

      if ($bits({32'h0040_0000 + 32'h8}) !== 32) begin
	 $display("FAILED -- bits count wrong");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
