
// This example is rediculous, but legal. However, Icarus Verilog will print
// various warnings about this. The warnings are OK, but Issue#576 saw this
// program assert, which is worse.
module test;
   function void fun;
      begin
	 $display("PASSED");
	 $finish;
      end
   endfunction // fun

   always_comb fun;

endmodule
