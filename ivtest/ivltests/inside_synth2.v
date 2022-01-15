module main;

   reg [7:0] th2, init;
   reg	     carry, clk,  rst, foo;

   (* ivl_synthesis_on *)
   always @(posedge clk) begin

      if (rst) begin
	 th2 <= 0;
	 carry <= 1;
	 foo <= 0; // This causes foo to be an output to the block.
      end else begin
	 if (carry)
	    {carry, th2} <= {1'b0, init};
	 else
	    {carry, th2} <= {1'b0, th2} + 9'h1;
      end

   end

   (* ivl_synthesis_off *)
   initial begin
      rst = 1;
      clk = 0;
      init = 8'hfe;
      $monitor("clk=%b:  rst=%b, th2=%h, carry=%b", clk, rst, th2, carry);

      #1 clk = 1;
      #1 clk = 0;
      if (foo !== 0) begin
	 $display("FAILED -- foo=%b", foo);
	 $finish;
      end

      rst = 0;

      #1 clk = 1;
      #1 clk = 0;

      #1 clk = 1;
      #1 clk = 0;
      if (th2 !== 8'hff) begin
	 $display("FAILED -- th2=%h (1)", th2);
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;
      if (th2 !== 8'h00) begin
	 $display("FAILED == th2=%h", th2);
	 $finish;
      end
      if (carry !== 1) begin
	 $display("FAILED -- carry=%b, th2=%h", carry, th2);
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;
      if (th2 !== 8'hfe) begin
	 $display("FAILED -- th2=%h", th2);
	 $finish;
      end

      #1 $strobe("PASSED");
   end // initial begin

endmodule // main
