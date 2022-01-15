
// This tests the basic support for default arguments to task/function
// ports. The default port syntax gives SystemVerilog a limited form
// of variable argument lists.

program main;

   function int increment(int val, int step = 1, int flag = 1);
      increment = val + step*flag;
   endfunction // increment

   initial begin

      if (increment(5) !== 6) begin
	 $display("FAILED -- increment(5) --> %0d", increment(5));
	 $finish;
      end

      if (increment(5,2) !== 7) begin
	 $display("FAILED -- increment(5,2) --> %0d", increment(5,2));
	 $finish;
      end

      if (increment(5,,3) !== 8) begin
	 $display("FAILED -- increment(5,,3) --> %0d", increment(5,,3));
	 $finish;
      end
      $display("PASSED");
   end

endprogram // main
