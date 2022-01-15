module main;

   reg clk;
   reg mem[1:0];
   reg clr;

   (* ivl_synthesis_on *)
   always @(posedge clk)
     if (clr) begin
	mem[1] <= 1;
	mem[0] <= 0;
     end else begin
	mem[1] <= ~mem[1];
	mem[0] <= ~mem[0];
     end

   (* ivl_synthesis_off *)
   initial begin
      clk = 0;
      clr = 1;
      #1 clk = 1;
      #1 clk = 0;

      if (mem[0] !== 0 || mem[1] !== 1) begin
	 $display("FAILED -- clr");
	 $finish;
      end

      clr = 0;
      #1 clk = 1;
      #1 clk = 0;

      if (mem[0] !== 1 || mem[1] !== 0) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
