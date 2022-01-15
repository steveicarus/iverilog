/*
 * This example is based on PR#584 in the bugs database.
 */

module main;
   reg clk;

always #50 clk = ~clk;

initial begin
   clk = 0;
   #100
     $display("%d", 1e3*2e-2);
     $display("%d", 1e2*0.2);
     $display("%d", 1e1*2); // prints ok
     $display("%d", 1e0*20.0); // prints ok
     $display("%d", 1e-1*200.0);
     // bug -- some correctly report "20" and others report "0"
     // looks like implicit real2integer conversion for every factor in expression
     // problem caused by partial support of reals
     $finish(0);
end
endmodule
