module main;

   reg [7:0] foo;
   reg [7:0] bar;

   initial begin
      foo = 'hff;

      if ($bits(foo) !== 8) begin
	 $display("FAILED -- $bits(foo) = %d", $bits(foo));
	 $finish;
      end

      if ($bits( 4'(foo) ) !== 4) begin
	 $display("FAILED -- $bits( 4'(foo) ) = %d", $bits( 4'(foo) ));
	 $finish;
      end

      bar = {4'd0, 4'(foo)};
      if (bar !== 8'h0f) begin
	 $display("FAILED -- foo=%b, bar=%b", foo, bar);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
