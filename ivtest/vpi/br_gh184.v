// When registering a simulation time callback, some simulators interpret
// the specified time value as relative to the current simulation time. To
// support this case, define the macro CB_TIME_IS_RELATIVE when compiling
// this module.

module main;

   integer val1, val2;

   initial begin
      val1 = 0;
      val2 = 1;
      #1;
      $poke_at_simtime(val1, 1, 10);
      $poke_at_simtime(val2, 2, 10);

`ifdef CB_TIME_IS_RELATIVE
      #1;
`endif
      #8;
      if (val1 !== 0) begin
	 $display("FAILED -- val1==%0d before delayed poke", val1);
	 $finish;
      end
      if (val2 !== 1) begin
	 $display("FAILED -- val2==%0d before delayed poke", val2);
	 $finish;
      end

      #1;
      if (val1 !== 1) begin
	 $display("FAILED -- val1==%0d: poke didn't happen", val1);
	 $finish;
      end
      if (val2 !== 2) begin
	 $display("FAILED -- val2==%0d: poke didn't happen", val2);
	 $finish;
      end

      $display("PASSED");
      $finish(0);
   end

endmodule // main
