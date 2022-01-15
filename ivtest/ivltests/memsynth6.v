module main;

   reg [7:0] mem [7:0], D;
   reg [3:0] radr, wadr;
   reg	     wr, clk;

   /*
    * This implements the synchronous write port to the memory.
    */
   always @(posedge clk)
     if (wr) mem[wadr] <= D;

   // This is the asynchronous read port from the memory.
   wire[7:0] Q = mem[radr];

   (* ivl_synthesis_off *)
   initial begin
      wr = 0;
      clk = 0;
      #1 clk = 1;
      #1 clk = 0;

      for (wadr = 0 ;  wadr < 8 ;  wadr = wadr+1) begin
	 wr = 1;
	 D = { 2{wadr} };
	 #1 clk = 1;
	 #1 clk = 0;
      end

      wr = 0;
      for (radr = 0 ;  radr < 8 ;  radr = radr+1) begin
	 #1 if (Q !==  {2{radr}}) begin
	    $display("FAILED -- mem[%d] == 'b%b", radr, Q);
	    $finish;
	 end
      end

      $display("PASSED");
   end
endmodule
