/*
 * This is a simple test for the for...join_none syntax. There is a
 * fork statement to start a bunch of threads. We wait for none of
 * them and instead watch them progress with the master thread.
 */
module main;

   int flag;
   initial begin
      flag = 0;
      fork
	 # 10 flag = 10;
	 # 20 flag = 20;
	 # 30 flag = 30;
      join_none

      #5 if (flag != 0) begin
	 $display("FAILED -- flag=%d (s.b. 0)", flag);
	 $finish;
      end

      #10 if (flag != 10) begin
	 $display("FAILED -- flag=%d (s.b. 10)", flag);
	 $finish;
      end

      #10 if (flag != 20) begin
	 $display("FAILED -- flag=%d (s.b. 20)", flag);
	 $finish;
      end

      #10 if (flag != 30) begin
	 $display("FAILED -- flag=%d (s.b. 30)", flag);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
