// Simple testbench 
module test;

   reg clock;
   reg rst_n;
   reg alw_high_sig;

   // Instantiate OVL example
   ovl_always u_ovl_always ( 
			     .clock     (clock),
			     .reset     (rst_n), 
			     .enable    (1'b1),
			     .test_expr (alw_high_sig)
			     );

   initial begin
      // Dump waves
      $dumpfile("dump.vcd");
      $dumpvars(1, test);

      // Initialize values.
      clock = 0;
      rst_n = 0;
      alw_high_sig = 0;

      $display("ovl_always does not fire at rst_n");
      alw_high_sig = 1;
      tick_clk();

      $display({"ovl_always does not fire ",
                "when alw_high_sig is FALSE"});
      rst_n = 1;
      alw_high_sig = 0;
      tick_clk();

      $display("ovl_always FIRES when test_exp is X");
      alw_high_sig = 1'bx;
      tick_clk();

      $display("ovl_always FIRES when test_exp is 1");
      alw_high_sig = 1'b1;
      tick_clk();

      $finish;
   end

   task tick_clk;
      repeat (2) #5 clock = ~clock;
   endtask
endmodule


