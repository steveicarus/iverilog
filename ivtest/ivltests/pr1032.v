module test;

   reg	d;
   wire bar;

   // Assign some value to bar with weak drive.
   assign (weak0, weak1) bar = d;

   // Whatever value is on bar, give that *strong* drive onto foo.
   // The strength of an assignment is its own, and does not come
   // from the strength contained in the r-value.
   tri0   foo = bar;

   initial begin
      d = 0;
      #1 if (d !== bar) begin
	 $display("FAILED -- d=%b, bar=%b", d, bar);
	 $finish;
      end

      if (d !== foo) begin
	 $display("FAILED -- d=%b, foo=%b", d, foo);
	 $finish;
      end

      d = 1;
      #1 if (d !== bar) begin
	 $display("FAILED -- d=%b, bar=%b", d, bar);
	 $finish;
      end

      if (d !== foo) begin
	 $display("FAILED -- d=%b, foo=%b", d, foo);
	 $finish;
      end

      d = 'bz;
      #1 if (d !== bar) begin
	 $display("FAILED -- d=%b, bar=%b", d, bar);
	 $finish;
      end

      if ('b0 !== foo) begin
	 $display("FAILED -- d=%b, foo=%b", d, foo);
	 $finish;
      end


      $display("PASSED");
      $finish;
   end // initial begin

endmodule // test
