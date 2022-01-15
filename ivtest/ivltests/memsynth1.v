/*
 * This program tests the synthesis of small memories, including
 * aysnchronous read w/ synchronous write.
 */
module main;

   reg [3:0] mem [1:0], D;
   reg	     rst, clk, wr, wadr, radr;

   /*
    * This implements the synchronous write port to the memory.
    */
   (* ivl_synthesis_on *)
   always @(posedge clk)
     if (rst) begin
	mem[0] <= 0;
	mem[1] <= 0;
     end else if (wr) begin
	mem[wadr] <= D;
     end

   /* This is the asynchronous read port from the memory. */
   wire [3:0] Q = mem[radr];

   (* ivl_synthesis_off *)
   initial begin
      rst = 0;
      clk = 0;
      wadr = 0;
      radr = 0;
      wr = 0;

      #1 clk = 1;
      #1 clk = 0;

      // Make sure reset works.
      rst = 1;

      #1 clk = 1;
      #1 clk = 0;

      #1 if (mem[0] !== 0 || mem[1] !== 0) begin
	 $display("FAILED -- Reset 1: mem[0]=%b, mem[1]=%b", mem[0], mem[1]);
	 $finish;
      end

      radr = 0;
      #1  if (Q !== mem[radr]) begin
	 $display("FAILED -- mem[%b] = %b, Q=%b", radr, mem[radr], Q);
	 $finish;
      end

      radr = 1;
      #1  if (Q !== mem[radr]) begin
	 $display("FAILED -- mem[%b] = %b, Q=%b", radr, mem[radr], Q);
	 $finish;
      end

      rst = 0;
      #1 clk = 1;
      #1 clk = 0;

      // Make sure memory remembers value.
      if (mem[0] !== 0 || mem[1] !== 0) begin
	 $display("FAILED -- Reset 2: mem[0]=%b, mem[1]=%b", mem[0], mem[1]);
	 $finish;
      end

      D = 7;
      wr = 1;
      #1 clk = 1;
      #1 clk = 0;

      // Make sure write works.
      if (mem[0] !== 7 || mem[1] !== 0) begin
	 $display("FAILED -- write D=%b: mem[0]=%b, mem[1]=%b",
		  D, mem[0], mem[1]);
	 $finish;
      end

      D = 2;
      wadr = 1;
      #1 clk = 1;
      #1 clk = 0;

      // Make sure write works.
      if (mem[0] !== 7 || mem[1] !== 2) begin
	 $display("FAILED -- write D=%b: mem[0]=%b, mem[1]=%b",
		  D, mem[0], mem[1]);
	 $finish;
      end

      radr = 0;
      #1  if (Q !== mem[radr]) begin
	 $display("FAILED -- mem[%b] = %b, Q=%b", radr, mem[radr], Q);
	 $finish;
      end

      wr = 0;
      D = 5;

      // Make sure memory remembers written values.
      if (mem[0] !== 7 || mem[1] !== 2) begin
	 $display("FAILED -- write D=%b: mem[0]=%b, mem[1]=%b",
		  D, mem[0], mem[1]);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
