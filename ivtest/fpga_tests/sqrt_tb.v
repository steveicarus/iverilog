 /*
  * This module is a test bench for the sqrt32 module. It runs some
  * test input values through the sqrt32 module, and checks that the
  * output is valid. If an invalid output is generated, print and
  * error message and stop immediately. If all the tested values pass,
  * then print PASSED after the test is complete.
  */
module main;

   reg [31:0] x;
   reg	      clk, reset;

   wire [15:0] y;
   wire        rdy;

   chip_root dut(.clk(clk), .reset(reset), .rdy(rdy), .x(x), .y(y));

   (* ivl_synthesis_off *)
   always #5 clk = !clk;

   task reset_dut;
      begin
	 reset = 1;
	 #1 reset = 0;
	 @(negedge clk) ;
      end
   endtask // reset_dut

   task crank_dut;
      begin
	 while (rdy == 0) begin
	    @(posedge clk) /* wait */;
	 end
      end
   endtask // crank_dut

   reg        GSR;
   assign      glbl.GSR = GSR;

   integer idx;

   (* ivl_synthesis_off *)
   initial begin
      reset = 0;
      clk = 0;

      /* If doing a post-map simulation, when we need to wiggle
         The GSR bit to simulate chip power-up. */
      GSR = 1;
      #100 GSR = 0;
      #100 x = 1;
      reset_dut;
      crank_dut;
      $display("x=%d, y=%d", x, y);

      x = 3;
      reset_dut;
      crank_dut;
      $display("x=%d, y=%d", x, y);

      x = 4;
      reset_dut;
      crank_dut;
      $display("x=%d, y=%d", x, y);

      for (idx = 0 ;  idx < 200 ;  idx = idx + 1) begin
	 x = $random;
	 reset_dut;
	 crank_dut;
	 $display("x=%d, y=%d", x, y);

	 if (x < (y * y)) begin
	    $display("ERROR: y is too big");
	    $finish;
	 end

	 if (x > ((y + 1)*(y + 1))) begin
	    $display("ERROR: y is too small");
	    $finish;
	 end
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
