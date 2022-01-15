
module main;

   int variable = 0;

   // A void function returns no value, so can be called
   // like a task, but without a warning about unused
   // results.
   function void test_incr(input int arg);
      variable = variable + arg;
   endfunction // test_incr

   initial begin
      variable = 0;
      test_incr(5);
      if (variable !== 5) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
   end

endmodule // main
