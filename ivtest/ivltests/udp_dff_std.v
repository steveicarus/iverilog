/*
 * udp_dff.v
 * This is based on a bug report from Chacko Neroth. The original
 * compalint was that the (b0) in the table generated a syntax error
 * message.
 */

// Same as udp_dff except use a standard UDP calling syntax.

module test;

   reg dd, clk, notifier;
   wire qq;
   dff d1 (qq, clk, dd, notifier);

   initial begin
      dd = 1'b0;
      clk= 0;

      #1 clk = 1;
      #1 if (qq !== 1'b0) begin
	 $display("FAILED -- D=%b, Q=%b", dd, qq);
	 $finish;
      end
      dd = 1'b1;

      clk = 0;
      #1 if (qq !== 1'b0) begin
	 $display("FAILED -- D=%b (hold 0), Q=%b", dd, qq);
	 $finish;
      end

      clk = 1;
      #1 if (qq !== 1'b1) begin
	 $display("FAILED -- D=%b, Q=%b", dd, qq);
	 $finish;
      end
      dd = 1'bx;

      clk = 0;
      #1 if (qq !== 1'b1) begin
	 $display("FAILED -- D=%b (hold 1), Q=%b", dd, qq);
	 $finish;
      end

      clk = 1;
      #1 if (qq !== 1'bx) begin
	 $display("FAILED -- D=%b, Q=%b", dd, qq);
	 $finish;
      end
      dd = 1'bz;

      clk = 0;
      #1 if (qq !== 1'bx) begin
	 $display("FAILED -- D=%b (hold x), Q=%b", dd, qq);
	 $finish;
      end

      clk = 1;
      #1 if (qq !== 1'bx) begin
	 $display("FAILED -- D=%b, Q=%b", dd, qq);
	 $finish;
      end
      dd = 1'b0;

      clk = 0;
      #1 if (qq !== 1'bx) begin
	 $display("FAILED -- D=%b (hold x), Q=%b", dd, qq);
	 $finish;
      end

      $display("PASSED");
   end
endmodule

primitive dff (Q, C, D, notifier);
   output Q;
   reg	  Q;
   input  C, D, notifier;
   table
      // C D notifier : Q : Q+
      (01) 0 ? : ? : 0 ; //normal clocking case
      (01) 1 ? : ? : 1 ; //normal clocking case
      (01) x ? : ? : x ; //normal clocking, input undefined
      (b0) ? ? : ? : - ; //clock falling or held low
      // if the above is changed to the following, it goes thru
      // (10) ? ? : ? : - ; //clock falling
      // but the following, which (b0) should iterate to fails also
      // (00) ? ? : ? : - ; //clock held low

      b (??) ? : ? : - ; //hold Q if D changes
      ? ? * : ? : x ; //notifier case
   endtable
endprimitive
