module top;

   reg [2:0] bar = 1;
   wire [3:0] foo;

   assign foo = { 1'b1, 0 ? 3'd0 : bar[2:0] };

   initial begin
      #1 if (foo !== 4'b1001) begin
	 $display("FAILED  bar=%b, foo=%b", bar, foo);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
