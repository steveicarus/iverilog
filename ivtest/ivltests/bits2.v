module main;

   reg  foo_reg;
   byte foo_byte;
   int  foo_int;
   shortint foo_shortint;
   longint  foo_longint;
   bit foo_bit;
   bit [13:0] foo14_bit;
   logic foo_logic;
   logic [10:0] foo11_logic;


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
     if ($bits(foo_bit) != 1) begin
	 $display("FAILED");
	 $finish;
     end
     if ($bits(foo14_bit) != 14) begin
	 $display("FAILED");
	 $finish;
     end
	 if ($bits(foo_logic) != 1) begin
	 $display("FAILED");
	 $finish;
     end
     if ($bits(foo11_logic) != 11) begin
	 $display("FAILED");
	 $finish;
     end
     $display("PASSED");

   end

endmodule // main
