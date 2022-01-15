module main;

   reg [2:0] Q;
   reg	     clk, clr, up;

   (*ivl_synthesis_off *)
   initial begin
      clk = 0;
      up = 0;
      clr = 1;

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 0) begin
	 $display("FAILED");
	 $finish;
      end

      up = 1;
      clr = 0;

      #1 clk = 1;
      #1 clk = 0;

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 3'b010) begin
	 $display("FAILED");
	 $finish;
      end

      up = 0;

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 3'b010) begin
	 $display("FAILED");
	 $finish;
      end

      clr = 1;

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 0) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

   /*
    * This statement models a snythesizable UP counter. The up
    * count is enabled by the up signal. The clr is an asynchronous
    * clear input.
    *
    * NOTE: This is bad style. Bad, bad style. It comes from a
    * customer's customer, so I try to support it, but I'll moan
    * about it. Much better (and clearer) is:
    *
    * if (clr)
    *   Q <= 0;
    * else
    *   Q <= Q+1;
    */
   (* ivl_synthesis_on *)
   always @(posedge clk, posedge clr) begin
      if (up)
	Q = Q + 1;
      if (clr)
	Q = 0;
   end

endmodule // main
