/*
 * This test is based on PR#913 in the test suite. The
 * idea is to wait for the start integer to become true.
 */
module test();
   integer start;

   initial
     #900000 begin $display("FAILED"); $stop; end

   initial
     #100000 start = 1;

   initial
     begin
	wait (start) $display("PASSED");
	$finish;
     end
endmodule // test
