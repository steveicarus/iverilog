/*
 * This program tests the synthesis of small memories, including
 * aysnchronous read w/ synchronous write.
 */
module main;

   reg [1:0] mem;
   reg	     D;
   reg	     rst, clk, wr, wadr, radr;

   /*
    * This implements the synchronous write port to the memory.
    * Asynchronous reset? In this case, yes, even though that is
    * not normally the case for RAM devices.
    */
   (* ivl_synthesis_on *)
   always @(posedge clk or posedge rst)
     if (rst) begin
	mem[0] <= 0;
	mem[1] <= 0;
     end else if (wr) begin
	mem[wadr] <= D;

     end else begin
     end

   /* This is the asynchronous read port from the memory. */
   wire Q = mem[radr];

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

      #1 if (mem[0] !== 0 || mem[1] !== 0) begin
	 $display("FAILED -- Reset: mem[0]=%b, mem[1]=%b", mem[0], mem[1]);
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
	 $display("FAILED -- Reset: mem[0]=%b, mem[1]=%b", mem[0], mem[1]);
	 $finish;
      end

      D = 1;
      wr = 1;
      #1 clk = 1;
      #1 clk = 0;

      // Make sure write works.
      if (mem[0] !== 1 || mem[1] !== 0) begin
	 $display("FAILED -- write D=%b: mem[0]=%b, mem[1]=%b",
		  D, mem[0], mem[1]);
	 $finish;
      end

      D = 0;
      wadr = 1;
      #1 clk = 1;
      #1 clk = 0;

      // Make sure write works.
      if (mem[0] !== 1 || mem[1] !== 0) begin
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
      D = 1;

      // Make sure memory remembers written values.
      if (mem[0] !== 1 || mem[1] !== 0) begin
	 $display("FAILED -- write D=%b: mem[0]=%b, mem[1]=%b",
		  D, mem[0], mem[1]);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
