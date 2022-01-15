module main;

   real foo;
   real bar;

   initial begin
      foo = 1.0;
      bar = 1.2;

      if (foo >= bar) begin
	 $display("FAILED -- foo < bar?");
	 $finish;
      end

      if (foo >= 1.2) begin
	 $display("FAILED -- foo < 1.2?");
	 $finish;
      end

      if (1.0 >= 1.2) begin
	 $display("FAILED -- 1.0 < 1.2?");
	 $finish;
      end

      if (1 >= 1.2) begin
	 $display("FAILED -- 1 < 1.2?");
	 $finish;
      end

      if (foo > bar) begin
	 $display("FAILED -- foo < bar?");
	 $finish;
      end

      if (foo > 1.2) begin
	 $display("FAILED -- foo < 1.2?");
	 $finish;
      end

      if (1.0 > 1.2) begin
	 $display("FAILED -- 1.0 < 1.2?");
	 $finish;
      end

      if (1 > 1.2) begin
	 $display("FAILED -- 1 < 1.2?");
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
