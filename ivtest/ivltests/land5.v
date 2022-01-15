module main;

   reg [1:0] a, b;
   reg	     flag;

   (* ivl_combinational *)
   always @(a, b)
     flag = a && b;

   (* ivl_synthesis_off *)
   initial begin
      a = 1;
      b = 0;
      #1 if (flag !== 0) begin
	 $display("FAILED -- a=%b, b=%b, flag=%b", a, b, flag);
	 $finish;
      end

      b = 2;
      #1 if (flag !== 1) begin
	 $display("FAILED -- a=%b, b=%b, flag=%b", a, b, flag);
	 $finish;
      end

      $display("PASSED");

   end

endmodule // main
