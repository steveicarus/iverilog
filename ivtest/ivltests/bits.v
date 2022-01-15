module main;

   reg  foo_reg;
   byte foo_byte;
   int  foo_int;
   shortint foo_shortint;
   longint  foo_longint;

   initial begin
      if ($bits(foo_reg) != 1) begin
	 $display("FAILED");
	 $finish;
      end
      if ($bits(foo_byte) != 8) begin
	 $display("FAILED");
	 $finish;
      end
      if ($bits(foo_int) != 32) begin
	 $display("FAILED");
	 $finish;
      end
      if ($bits(foo_shortint) != 16) begin
	 $display("FAILED");
	 $finish;
      end
      if ($bits(foo_longint) != 64) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
