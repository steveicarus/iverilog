module top;
   reg [1:0] q;
   reg [4:0] icim[1:0];
   integer   j;

   always @(q) begin
      /*
       * The following line had the muli problem, the other line has a
       * different problem.
       */
      icim[0] <= #1 0 + 8 * (0 >> q);
      icim[1] <= #1 1 + 8 * (1 >> q);
   end

   initial begin
      q = 2'd1;
      #2;
      if (icim[0] !== 0) begin
	 $display("FAILED");
	 $finish;
      end
      if (icim[1] !== 1) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
  end

endmodule
