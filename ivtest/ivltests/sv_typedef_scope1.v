
typedef struct packed {
   logic [1:0] hig;
   logic [1:0] low;
} foo_t;

module main;
   // This typedef should work, and should shadow the foo_t definition
   // in the $root scope.
   typedef struct packed {
      logic [2:0] hig_x;
      logic [2:0] low_x;
   } foo_t;

   foo_t foo;

   initial begin
      foo = 6'b111000;

      if ($bits(foo_t) != 6) begin
	 $display("FAILED -- Got wrong foo_t definition?");
	 $finish;
      end

      if ($bits(foo) != 6) begin
	 $display("FAILED -- $bits(foo)==%0d", $bits(foo));
	 $finish;
      end

      if (foo.hig_x !== 3'b111) begin
	 $display("FAILED -- foo=%b, foo.hig_x=%b", foo, foo.hig_x);
	 $finish;
      end

      if (foo.low_x !== 3'b000) begin
	 $display("FAILED -- foo=%b, foo.low_x=%b", foo, foo.low_x);
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
