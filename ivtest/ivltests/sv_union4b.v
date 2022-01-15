
typedef union packed {
   logic [3:0] bits;
   struct      packed { logic [1:0] hig; logic [1:0] low; } words;
} bits_t;

module main;

   bits_t foo;

   initial begin

      if ($bits(foo) !== 4) begin
	 $display("FAILED -- $bits(foo)=%0d", $bits(foo));
	 $finish;
      end

      if ($bits(bits_t) !== 4) begin
	 $display("FAILED -- $bits(bits_t)=%0d", $bits(bits_t));
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
