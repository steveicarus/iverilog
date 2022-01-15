/*
 * In this example, the set and clr are both synchronous. This checks
 * that this complex case is handled correctly.
 */
module main;


   reg	      Q, clk, rst, set, clr;
   (* ivl_synthesis_on *)
   always@(posedge clk or posedge rst)
     begin
	if (rst)
	  Q <= 1'b0;
	else if (set)
	  Q <= 1'b1;
	else if (clr)
	  Q <= 1'b0;
     end

   (* ivl_synthesis_off *)
   initial begin
      clk = 0;
      rst = 0;
      set = 0;
      clr = 0;

      #1 rst = 1;
      #1 rst = 0;

      if (Q !== 0) begin
	 $display("FAILED -- rst");
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 0) begin
	 $display("FAILED -- 1 clk");
	 $finish;
      end

      #1 set = 1;
      #1 ;

      if (Q !== 0) begin
	 $display("FAILED -- 1 set (no clk)");
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 1) begin
	 $display("FAILED -- 1 set");
	 $finish;
      end

      #1 clr = 1;
      #1 ;

      if (Q !== 1) begin
	 $display("FAILED -- 1 clr+set (no clk)");
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 1) begin
	 $display("FAILED -- 1 clr+set");
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 1) begin
	 $display("FAILED -- 2 clr+set");
	 $finish;
      end

      #1 set = 0;
      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 0) begin
	 $display("FAILED -- 1 clr-set");
	 $finish;
      end

      #1 clr = 0;
      #1 clk = 1;
      #1 clk = 0;

      if (Q !== 0) begin
	 $display("FAILED -- 1 set-clr");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule
