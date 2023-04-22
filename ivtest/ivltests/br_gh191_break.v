//
// github Issue#191
//
module main;
   integer idx;
   initial begin
      for(idx=0; idx < 5; idx=idx+1 ) begin
	 if (idx >= 2)
	   break;
      end
      if (idx != 2) begin
	 $display("FAILED -- break from for loop");
	 $finish;
      end

      idx = 0;
      forever begin
	 idx += 1;
	 if (idx >= 2)
	   break;
      end
      if (idx != 2) begin
	 $display("FAILED -- break from forever loop");
	 $finish;
      end

      idx = 0;
      while (idx < 5) begin
	 if (idx >= 2)
	   break;
	 idx += 1;
      end
      if (idx != 2) begin
	 $display("FAILED -- break from while loop");
	 $finish;
      end

      idx = 0;
      do begin
	 if (idx >= 2)
	   break;
	 idx += 1;
      end while (idx < 5);

      if (idx != 2) begin
	 $display("FAILED -- break from do-while loop");
	 $finish;
      end

      idx = 0;
      repeat (5) begin
	 if (idx >= 2)
	   break;
	 idx += 1;
      end
      if (idx != 2) begin
	 $display("FAILED -- break from repeat(5) loop");
	 $finish;
      end

      $display("PASSED");
   end
endmodule
