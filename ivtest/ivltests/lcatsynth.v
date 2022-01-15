module main;

   reg q0, q1, clk, clr;

   (* ivl_synthesis_on *)
   always @(posedge clk, posedge clr)
     if (clr) begin
	//q0 <= 0;
	//q1 <= 0;
	{q0, q1} <= 2'b00;
     end else begin
       {q1, q0} <= {q1, q0} + 1;
     end

   (* ivl_synthesis_off *)
   initial begin
      clk = 0;
      clr = 1;
      #1 clk = 1;
      #1 clk = 0;
      if ({q1,q0} !== 2'b00) begin
	 $display("FAILED");
	 $finish;
      end

      clr = 0;

      #1 clk = 1;
      #1 clk = 0;

      if ({q1,q0} !== 2'b01) begin
	 $display("FAILED");
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;

      if ({q1,q0} !== 2'b10) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
