// Check that VAMS `abs()` functions works if its argument is a function call

module main;

   function reg signed [7:0] fv(input reg signed [7:0] x);
      fv = x;
   endfunction

   function real fr(input real x);
      fr = x;
   endfunction

   reg signed [7:0]  a;
   wire signed [7:0] vala = abs(fv(a));

   reg	      real  b;
   wire       real  valb = abs(fr(b));

   initial begin
      a = 0;
      b = 0;
      #1 if (vala !== 0) begin
	 $display("FAILED -- a=%b, vala=%b", a, vala);
	 $finish;
      end

      #1 if (valb != 0) begin
	 $display("FAILED -- b=%g valb=%g", b, valb);
	 $finish;
      end

      a = 1;
      b = 1;
      #1 if (vala !== 1) begin
	 $display("FAILED -- a=%b, vala=%b", a, vala);
	 $finish;
      end

      #1 if (valb != 1) begin
	 $display("FAILED -- b=%g valb=%g", b, valb);
	 $finish;
      end

      a = -1;
      b = -1;
      #1 if (vala !== 1) begin
	 $display("FAILED -- a=%b, vala=%b", a, vala);
	 $finish;
      end

      #1 if (valb != 1) begin
	 $display("FAILED -- b=%g valb=%g", b, valb);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
