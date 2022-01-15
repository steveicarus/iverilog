module main;

   reg [7:0] a;
   reg [2:0] adr, w_adr;
   reg	     rst, clk, ae, wr;

   (* ivl_synthesis_on *)
   always @(posedge clk)
     if (rst) begin
	a   <= 8'b00000000;
	adr <= 3'b000;
     end else if (ae) begin
	adr <= w_adr;
     end else if (wr) begin
	adr <= adr + 1;
	a[adr] <= 1;
     end

   (* ivl_synthesis_off *)
   initial begin

      clk = 0;
      wr = 0;
      ae = 0;
      rst = 1;
      #1 clk = 1;
      #1 clk = 0;
      if (a !== 8'b0000_0000 || adr !== 3'b000) begin
	 $display("FAILED - reset - a=%b, adr=%b", a, adr);
	 $finish;
      end

      rst = 0;
      #1 clk = 1;
      #1 clk = 0;
      if (a !== 8'b0000_0000 || adr !== 3'b000) begin
	 $display("FAILED - pause - a=%b, adr=%b", a, adr);
	 $finish;
      end

      wr = 1;
      #1 clk = 1;
      #1 clk = 0;
      if (a !== 8'b0000_0001 || adr !== 3'b001) begin
	 $display("FAILED - wr 1 - a=%b, adr=%b", a, adr);
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;
      if (a !== 8'b0000_0011 || adr !== 3'b010) begin
	 $display("FAILED - wr 2 - a=%b, adr=%b", a, adr);
	 $finish;
      end

      ae = 1;
      w_adr = 4;
      #1 clk = 1;
      #1 clk = 0;
      if (a !== 8'b0000_0011 || adr !== 3'b100) begin
	 $display("FAILED - ae - a=%b, adr=%b", a, adr);
	 $finish;
      end

      ae = 0;
      #1 clk = 1;
      #1 clk = 0;
      if (a !== 8'b0001_0011 || adr !== 3'b101) begin
	 $display("FAILED - ae - a=%b, adr=%b", a, adr);
	 $finish;
      end

      $display("PASSED");

   end

endmodule // main
