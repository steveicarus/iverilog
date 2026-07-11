// Extended VCD: $dumpportsoff suspends (X checkpoint), $dumpportson resumes,
// $dumpportsall forces a checkpoint.
module evcd_onoff_dut (input wire clk, input wire d, output wire q);
   reg r;
   assign q = r;
   always @(posedge clk) r <= d;
endmodule

module evcd_onoff;
   reg  clk, d;
   wire q;
   evcd_onoff_dut u (.clk(clk), .d(d), .q(q));
   always #5 clk = ~clk;
   initial begin
      $dumpports(u, "work/evcd_onoff.evcd");
      clk = 0; d = 0;
      #7  d = 1;
      #6  $dumpportsoff;
      #10 d = 0;
      #2  $dumpportson;
      #10 $dumpportsall;
      #5  $finish;
   end
endmodule
