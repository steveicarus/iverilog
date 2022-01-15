
typedef enum { A, B } E;

module main;
   E in;
   E out;

   function E func(input E in);
      func = in;
   endfunction

   initial begin
      out = func(A);
      #1 if (out !== A) begin
	 $display("FAIL: in=%0d, out=%0d", in, out);
	 $finish;
      end

      out = func(B);
      #1 if (out !== B) begin
	 $display("FAIL: in=%0d, out=%0d", in, out);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endmodule // main
