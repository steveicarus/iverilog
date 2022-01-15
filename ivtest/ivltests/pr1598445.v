module test();

   wire sig;
   reg	en0, en1;

   pullup (sig);
   assign sig = en0 ? 1'b0 : 1'bz;
   assign sig = en1 ? 1'b1 : 1'bz;

   reg	sig2;
   always @(sig)
     sig2 = sig;

   initial begin

      en0 = 0;
      en1 = 1;
      #1 en1 = 0;

      #1 if (sig2 !== 1'b1) begin
	 $display("FAILED -- sig2=%b, sig=%b", sig2, sig);
	 $finish;
      end

      force sig = 0;

      #1 if (sig2 !== 1'b0) begin
	 $display("FAILED -- sig2=%b, sig=%b", sig2, sig);
	 $finish;
      end

      release sig;

      #1 if (sig2 !== 1'b1) begin
	 $display("FAILED -- sig2=%b, sig=%b", sig2, sig);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
