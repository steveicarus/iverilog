module main;

   wire [3:0] b = 4'b1111;
   wire [3:0] c = 4'b1111;

   initial begin
      #0; // avoid time-0 race
      $display("%b",  ((c & ~(1'b1<<9'h00)) & b)); // s.b. 1110
      $display("%b", |((c & ~(1'b1<<9'h00)) & b)); // s.b. 1

      if ( ((c & ~(1'b1<<9'h00)) & b) !== 4'b1110) begin
	 $display("FAILED (1)");
	 $finish;
      end

      if (|((c & ~(1'b1<<9'h00)) & b) !== 1'b1) begin
	 $display("FAILED (2)");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
