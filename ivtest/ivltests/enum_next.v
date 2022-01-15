/*
 * This program tests that enumeration value first/last/next
 * methods work properly. The .next method requires some run-time
 * support for enumeration.
 */
module main;

   enum { RED, GREEN = 2, BLUE } color1;

   initial begin
      color1 = RED;
      $display("color1.first == %0d", color1.first);
      $display("color1.last  == %0d", color1.last);
      $display("color1.num   == %0d", color1.num);
      $display("color1.next  == %0d", color1.next);

      color1 = color1.next;
      if (color1 != GREEN || color1 !== 2) begin
	 $display("FAILED -- should be %0d, got %0d", GREEN, color1);
	 $finish;
      end

      color1 = color1.next;
      if (color1 != BLUE || color1 !== 3 || color1 != color1.last) begin
	 $display("FAILED -- should be %0d, got %0d", BLUE, color1);
	 $finish;
      end

      color1 = color1.prev;
      if (color1 != GREEN || color1 !== 2) begin
	 $display("FAILED -- should be %0d, got %0d", GREEN, color1);
	 $finish;
      end

      color1 = color1.prev;
      if (color1 != RED || color1 !== 0 || color1 != color1.first) begin
	 $display("FAILED -- should be %0d, got %0d", RED, color1);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
