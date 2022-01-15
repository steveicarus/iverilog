module main;

   reg [23:20] foo;
   wire [3:0]  test = {foo[22:20] == 3'd0, foo[22:20]};

   initial begin

      foo = 4'b1_000;
      #1 if (test !== 4'b1_000) begin
	 $display("FAILED -- foo=%b, test=%b", foo, test);
	 $finish;
      end

      foo = 4'b0_111;
      #1 if (test !== 4'b0_111) begin
	 $display("FAILED -- foo=%b, test=%b", foo, test);
	 $finish;
      end

      foo = 4'b0_000;
      #1 if (test !== 4'b1_000) begin
	 $display("FAILED -- foo=%b, test=%b", foo, test);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
