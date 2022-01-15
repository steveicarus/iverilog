// When registering a simulation time callback, some simulators interpret
// the specified time value as relative to the current simulation time. To
// support this case, define the macro CB_TIME_IS_RELATIVE when compiling
// this module.

module main;

   integer val;

   initial begin
      val = 0;
      #1 $poke_at_simtime(val, 1, 10);

`ifdef CB_TIME_IS_RELATIVE
      #1;
`endif
      #8 if (val !== 0) begin
	 $display("FAILED -- val==%0d before delayed poke", val);
	 $finish;
      end

      #1 if (val !== 1) begin
	 $display("FAILED -- val==%0d: poke didn't happen", val);
	 $finish;
      end

      $display("PASSED");
      $finish(0);
   end

endmodule // main
