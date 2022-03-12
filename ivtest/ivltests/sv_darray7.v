// Check that dynamic arrays of packed array types are supported

module main;

  typedef reg [3:0] T1;
  typedef T1 [7:0] T2;

   T2 foo[];
   int idx;

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

      for (idx = 0 ; idx < foo.size() ; idx += 1) begin
	 foo[idx] = idx;
      end

      $display("foo[7] = %d", foo[7]);
      if (foo[7] != 7) begin
	 $display("FAILED -- foo[7] = %0d (s.b. 7)", foo[7]);
	 $finish;
      end

      $display("foo[9] = %d", foo[9]);
      if (foo[9] != 9) begin
	 $display("FAILED -- foo[9] = %0d (s.b. 9)", foo[9]);
	 $finish;
      end

      for (idx = 0 ; idx < 2*foo.size() ; idx += 1) begin
	 if (foo[idx%10] != (idx%10)) begin
	    $display("FAILED -- foo[%0d%%10] = %0d", idx, foo[idx%10]);
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
