module main;

   reg a, b, c;

   reg clk, rst, rnd;

   (* ivl_sinthesis_on *)
   always @(posedge clk or posedge rst)
     if (rst) begin
	a <= 0;
	b <= 0;
	c <= 0;
     end else if (rnd) begin
	a <= 0;
	b <= 0;
     end else begin
	{c, b, a} <= {c, b, a} + 3'b001;
     end

   (* ivl_synthesis_off *)
   initial begin
      clk = 0;
      rst = 0;
      rnd = 0;
      #1 rst = 1;
      #1 rst = 0;
      if ({c,b,a} !== 3'b000) begin
	 $display("FAILED - no async reset");
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;
      if ({c,b,a} !== 3'b001) begin
	 $display("FAILED - First clock failed. {%b,%b,%b}", c, b, a);
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;
      #1 clk = 1;
      #1 clk = 0;
      #1 clk = 1;
      #1 clk = 0;
      #1 clk = 1;
      #1 clk = 0;
      if ({c,b,a} !== 3'b101) begin
	 $display("FAILED - Fifth clock failed. {%b,%b,%b}", c, b, a);
	 $finish;
      end

      rnd = 1;
      #1 clk = 1;
      #1 clk = 0;
      if ({c,b,a} !== 3'b100) begin
	 $display("FAILED - rnd failed. {%b,%b,%b}", c, b, a);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
