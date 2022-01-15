
typedef struct packed {
   logic       b;
} single_bit;

typedef struct packed {
   single_bit b1;
   single_bit b2;
} two_bits;

module simple(input two_bits b2in,
              output two_bits b2out);
   assign b2out.b1.b = b2in.b1.b;
endmodule // simple

module main;

   two_bits src;
   wire two_bits dst;

   simple copy(src, dst);
   assign dst.b2.b = src.b2.b;

   initial begin
      src.b1.b = 1'b1;
      src.b2.b = 1'b0;
      #1 ; // Let values settle.
      $display("src=%b (s.b. 10), dst=%b (s.b. 10)", src, dst);
      if (src !== 2'b10 || dst !== 2'b10) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
      $finish;
   end

endmodule // main
