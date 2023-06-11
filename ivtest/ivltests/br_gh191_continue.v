//
// github Issue#191
//
module main;
   integer idx;
   initial begin

      for(idx=0; idx < 5; idx=idx+1 ) begin
	 if (idx < 2)
	   continue;
	 if (idx < 2) begin
	    $display("FAILED -- continue in for-loop");
	    $finish;
	 end
      end
      if (idx != 5) begin
	 $display("FAILED -- break from for loop");
	 $finish;
      end

      idx = 0;
      forever begin
	 idx += 1;
	 if (idx < 2)
	   continue;
	 if (idx < 2) begin
	    $display("FAILED -- continue in forever-loop");
	    $finish;
	 end
	 // Need a 'break', since that (and 'return') is the only
	 // way to escape a 'forever' loop.
	 break;
      end
      if (idx != 2) begin
	 $display("FAILED -- break from forever loop");
	 $finish;
      end

      idx = 0;
      while (idx < 5) begin
	 idx += 1;
	 if (idx < 2)
	   continue;
	 if (idx < 2) begin
	    $display("FAILED -- continue in while loop");
	    $finish;
	 end
      end
      if (idx != 5) begin
	 $display("FAILED -- break from while loop");
	 $finish;
      end

      idx = 0;
      do begin
	 idx += 1;
	 if (idx < 2)
	   continue;
	 if (idx < 2) begin
	    $display("FAILED -- continue in do-while loop");
	    $finish;
	 end
      end while (idx < 5);
      if (idx != 5) begin
	 $display("FAILED -- break from do-while loop");
	 $finish;
      end

      idx = 0;
      repeat (5) begin
	 idx += 1;
	 if (idx < 2)
	   continue;
	 idx += 1;
      end
      if (idx != 9) begin
	 $display("FAILED -- continue from repeat(5) loop (idx=%0d)", idx);
	 $finish;
      end

      $display("PASSED");
   end
endmodule
