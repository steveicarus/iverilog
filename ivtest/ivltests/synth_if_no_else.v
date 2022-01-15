
module test(output reg Q, input wire D, input wire OE);

   always @* begin
      Q = 0;
      if (OE)
	Q = D;
   end
endmodule // test

module main;
   reg D, OE;
   wire Q;

   test dut(Q, D, OE);

   (* ivl_synthesis_off *) initial begin
      OE = 0;
      D = 0;
      #1 if (Q !== 0) begin
	 $display("FAILED -- Q=%b, D=%b, OE=%b", Q, D, OE);
	 $finish;
      end

      D = 1;
      #1 if (Q !== 0) begin
	 $display("FAILED -- Q=%b, D=%b, OE=%b", Q, D, OE);
	 $finish;
      end

      OE = 1;
      #1 if (Q !== 1) begin
	 $display("FAILED -- Q=%b, D=%b, OE=%b", Q, D, OE);
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
