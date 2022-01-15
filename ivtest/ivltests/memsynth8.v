module main;

   reg [7:0] mem;
   reg [2:0] addr;
   reg	     out;
   reg	     clk;

   (* ivl_synthesis_on *)
   always @(posedge clk) out <= mem[addr];

   integer idx;
   (* ivl_synthesis_off *)
   initial begin
      mem = 8'hca;
      addr = 0;
      clk = 0;

      for (idx = 0 ;  idx < 8 ;  idx = idx+1) begin
	 addr = idx[2:0];
	 #1 clk = 1;
	 #1 clk = 0;
	 if (out !== mem[idx]) begin
	    $display("FAILED -- mem[%d] = %b", idx, out);
	    $finish;
	 end
      end

      $display("PASSED");
   end // initial begin

endmodule // main
