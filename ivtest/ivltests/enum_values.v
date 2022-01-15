/*
 * This program tests that enumeration values work and are
 * implicitly translated to integer values.
 */
module main;

   enum { RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET,
	  BLACK = 10, WHITE = 'd11
	} color1;

   int	var1;

   initial begin
      color1 = RED;
      var1 = RED;
      $display("color1 = %0d, var1 = %0d", color1, var1);
      if (color1 !== 0) begin
	 $display("FAILED");
	 $finish;
      end
      if (var1 !== 0) begin
	 $display("FAILED");
	 $finish;
      end

      color1 = GREEN;
      var1 = GREEN;
      $display("color1 = %0d, var1 = %0d", color1, var1);
      if (color1 !== 3) begin
	 $display("FAILED");
	 $finish;
      end
      if (var1 !== 3) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
