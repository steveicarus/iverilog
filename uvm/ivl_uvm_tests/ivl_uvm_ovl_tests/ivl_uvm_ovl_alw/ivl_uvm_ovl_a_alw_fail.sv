`timescale 1ns/1ns

module test;

   wire clk;
   reg rst_n;
   reg alw_high_sig;

   // Instantiate OVL example
   ovl_always u_ovl_always ( 
			     .clock     (clk),
			     .reset     (rst_n), 
			     .enable    (1'b1),
			     .test_expr (alw_high_sig)
			     );

   initial begin
      // Dump waves
      $dumpfile("dump.vcd");
      $dumpvars(1, test);

      // Initialize values.
      rst_n = 0;
      alw_high_sig = 0;

      $display("ovl_always does not fire at rst_n");
      alw_high_sig = 1;
      wait_clks(5);

      rst_n = 1;
      wait_clks(5);
      $display("Out of reset");

      alw_high_sig = 0;
      $display({"ovl_always expect 1- OVL_ERROR ",
                "when alw_high_sig is FALSE"});


      wait_clks(10);

      $finish;
   end

   task wait_clks (input int num_clks = 1);
      repeat (num_clks) @(posedge clk);
   endtask : wait_clks

  ivl_uvm_ovl_clk_gen #(.FREQ_IN_MHZ(100)) u_clk_100 (clk);

endmodule

