
// This tests the basic support for default arguments to task/function
// ports. The default port syntax gives SystemVerilog a limited form
// of variable argument lists.

program main;

   task increment(output int res, input int val, input int step = 1, input int flag = 1);
      res = val + step*flag;
   endtask // increment

   int res;
   initial begin

      increment(res,5);
      if (res !== 6) begin
	 $display("FAILED -- increment(5) --> %0d", res);
	 $finish;
      end

      increment(res,5,2);
      if (res !== 7) begin
	 $display("FAILED -- increment(5,2) --> %0d", res);
	 $finish;
      end

      increment(res,5,,3);
      if (res !== 8) begin
	 $display("FAILED -- increment(5,,3) --> %0d", res);
	 $finish;
      end
      $display("PASSED");
   end

endprogram // main
