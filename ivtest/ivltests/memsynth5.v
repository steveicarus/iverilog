module main;

   reg [7:0] mem [7:0], D;
   reg [2:0] radr, wadr;
   reg	     wr, rst, clk;

   /*
    * This implements the synchronous write port to the memory.
    */
   always @(posedge clk)

     if (rst) begin
	mem[0] <= 0;
	mem[1] <= 0;
	mem[2] <= 0;
	mem[3] <= 8'h33;
	mem[5] <= 8'h55;
	mem[6] <= 0;
	mem[7] <= 0;
     end else
       if (wr) begin
	  mem[wadr] <= D;
       end

   // This is the asynchronous read port from the memory.
   wire[7:0] Q = mem[radr];

   initial begin
      wr = 0;
      rst = 1;
      clk = 0;
      #1 clk = 1;
      #1 clk = 0;

      radr = 3;
      #1 if (Q !== 8'h33) begin
	 $display("FAILED -- mem[3] == 'b%b", Q);
	 $finish;
      end

      radr = 5;
      #1 if (Q !== 8'h55) begin
	 $display("FAILED == mem[5] == 'b%b", Q);
	 $finish;
      end

      wadr = 4;
      wr = 1;
      rst = 0;
      D = 'h44;
      #1 clk = 1;
      #1 clk = 0;

      radr = 4;
      #1 if (Q !== 8'h44) begin
	 $display("FAILED -- mem[4] == 'b%b", Q);
	 $finish;
      end

      $display("PASSED");
   end
endmodule
