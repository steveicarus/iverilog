// This tests that the individual bits of a uwire are checked for
// double-driving individually. The code below is legal.
module test;

   uwire [31:0] foo;

   assign foo[15:0] = 'haaaa;
   assign foo[31:16] = 'h5555;

   initial begin
      #1 if (foo !== 'h5555aaaa) begin
	 $display("FAILED -- foo=%h", foo);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // test
