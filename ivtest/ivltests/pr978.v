/*
 * This test is based on PR#978.
 * Check that user defined functions can have real-valued
 * results, and that the result gets properly returned.
 */
module test();
   real m;

   function real dummy;
      input b;
      begin
	 dummy=2.5;
      end
   endfunction

   initial
     begin
	m=dummy(0);

	if (m != 2.5) begin
	   $display("FAILED: return result is %f", m);
	   $finish;
	end

	$display("PASSED");
     end

endmodule
