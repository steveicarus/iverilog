module main;

   reg clk;
   reg Q, D, ce;

   (* ivl_synthesis_on *)
   always @(posedge clk)
     if (ce)
       begin
       end
     else
       Q <= D;

   (* ivl_synthesis_off *)
   initial begin
      clk = 0;
      ce = 0;
      D = 0;
      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 1'b0) begin
	 $display("FAILED --- initial setup failed: Q=%b, D=%b, ce=%b",
		  Q, D, ce);
	 $finish;
      end

      ce = 1;
      D = 1;
      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 1'b0) begin
	 $display("FAILED --- disable didnot work: Q=%b, D=%b, ce=%b",
		  Q, D, ce);
	 $finish;
      end

      ce = 0;
      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 1'b1) begin
	 $display("FAILED --- disabled disable not OK: Q=%b, D=%b, ce=%b",
		  Q, D, ce);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
