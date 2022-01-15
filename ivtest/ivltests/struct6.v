// This tests that the individual bits of a uwire are checked for
// double-driving individually. The code below uses a packed struct
// to represent individual bits.
module test;

   struct packed {
      logic [15:0] hig;
      logic [15:0] low;
   } foo;

   assign foo.low = 'haaaa;
   assign foo.hig = 'h5555;

   initial begin
      #1 if (foo !== 'h5555aaaa) begin
	 $display("FAILED -- foo=%h", foo);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // test
