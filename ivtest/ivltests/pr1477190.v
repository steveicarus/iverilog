module main;

   reg b, a;

   initial begin
      b = 0;
      a = 1;
      #1 if (b !== 0) begin
	 $display("FAILED -- b starts out as %b", b);
	 $finish;
      end

      force b = a;
      #1 if (b !== 1) begin
	 $display("FAILED -- b=%b, a=%b", b, a);
	 $finish;
      end

      a = 0;
      #1 if (b !== 0) begin
	 $display("FAILED -- b=%b, a=%b", b, a);
	 $finish;
      end

      a = 1;
      #1 release b;
      #1 a = 0;
      #1 if (b !== 1) begin
	 $display("FAILED -- b=%b didnot hold value after release", b);
	 $finish;
      end

      b = 0;
      if (b !== 0) begin
	 $display("FAILED -- assign failed b=%b", b);
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
