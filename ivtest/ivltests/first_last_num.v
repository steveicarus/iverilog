/*
 * This program tests that enumeration value first/last/num
 * methods work properly.
 */
module main;

   enum byte { RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET,
	  BLACK = 10, WHITE = 'd11
	} color1;


   initial begin
      color1 = RED;
      $display("color1.first == %0d", color1.first);
      $display("color1.last  == %0d", color1.last);
      $display("color1.num   == %0d", color1.num);

      if (color1.first !== RED) begin
	 $display("FAILED");
	 $finish;
      end

      if (color1.last !== WHITE) begin
	 $display("FAILED");
	 $finish;
      end

      if (color1.num !== 8) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
