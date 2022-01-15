
module main;

   int variable = 0;

   // A void function returns no value, so can be called
   // like a task, but without a warning about unused
   // results. It does not have to take any arguments.
   function void test_incr();
      variable = variable + 1;
   endfunction // test_incr

   initial begin
      variable = 0;
      test_incr();
      if (variable !== 1) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
   end

endmodule // main
