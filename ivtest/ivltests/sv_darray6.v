
/*
 * This demonstrates a basic dynamic array
 */
module main;

   string foo[];
   int idx;

   string tmp;
   initial begin
      if (foo.size() != 0) begin
	 $display("FAILED -- foo.size()=%0d, s.b. 0", foo.size());
	 $finish;
      end

      foo = new[10];
      if (foo.size() != 10) begin
	 $display("FAILED -- foo.size()=%0d, s.b. 10", foo.size());
	 $finish;
      end

      tmp = "fooa";
      for (idx = 0 ; idx < foo.size() ; idx += 1) begin
	 tmp[3] = 'h41 + idx;
	 foo[idx] = tmp;
      end

      $display("foo[7] = %0s", foo[7]);
      if (foo[7] != "fooH") begin
	 $display("FAILED -- foo[7] = %0s (s.b. fooH)", foo[7]);
	 $finish;
      end

      $display("foo[9] = %0s", foo[9]);
      if (foo[9] != "fooJ") begin
	 $display("FAILED -- foo[9] = %0s (s.b. fooJ)", foo[9]);
	 $finish;
      end

      for (idx = 0 ; idx < 2*foo.size() ; idx += 1) begin
	 tmp[3] = 'h41 + (idx%10);
	 if (foo[idx%10] != tmp) begin
	    $display("FAILED -- foo[%0d%%10] = %0s", idx, foo[idx%10]);
	    $finish;
	 end
      end

      foo.delete();
      if (foo.size() != 0) begin
	 $display("FAILED -- foo.size()=%0d (after delete: s.b. 0)", foo.size());
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
