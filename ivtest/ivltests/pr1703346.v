module main;

   wire [1:0] foo [0:1];

   assign     (highz0, strong1) foo[0] = 2'b01;
   assign     (strong0, highz1) foo[0] = 2'b01;

   assign     (highz0, strong1) foo[1] = 2'b10;
   assign     (strong0, highz1) foo[1] = 2'b10;

   initial begin
      #1 $display("foo[0] = %b, foo[1] = %b", foo[0], foo[1]);

      if (foo[0] !== 2'b01) begin
	 $display("FAILED -- foo[0] = %b", foo[0]);
	 $finish;
      end

      if (foo[1] !== 2'b10) begin
	 $display("FAILED == foo[1] = %b", foo[1]);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
