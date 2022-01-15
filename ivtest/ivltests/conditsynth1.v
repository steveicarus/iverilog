module main;

   reg [2:0] Q;
   reg	     clk, clr, up, down;

   (*ivl_synthesis_off *)
   initial begin
      clk = 0;
      up = 0;
      down = 0;
      clr = 1;

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 0) begin
	 $display("FAILED");
	 $finish;
      end

      up = 1;
      clr = 0;

      #1 clk = 1;
      #1 clk = 0;

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 3'b010) begin
	 $display("FAILED");
	 $finish;
      end

      up = 0;
      down = 1;

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 3'b001) begin
	 $display("FAILED");
	 $finish;
      end

      down = 0;

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 3'b001) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

   /*
    * This statement models a snythesizable UP/DOWN counter. The up
    * and down cases are enabled by up and down signals. If both
    * signals are absent, the synthesizer should take the implicit
    * case that Q <= Q;
    */
   (* ivl_synthesis_on *)
   always @(posedge clk, posedge clr)
     if (clr) begin
       Q <= 0;
     end else begin
	if (up)
	  Q <= Q + 1;
	else if (down)
	  Q <= Q - 1;
     end

endmodule // main
