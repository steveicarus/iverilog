module top;
  lower #(1, 2, 3) dut();
endmodule

module lower;
   parameter one = 1;                  // This should be 'sd1
   parameter two = 2;                  // This should be 'sd2
   parameter three = 0;                // This should be 'sd3
   parameter local1 = one - two;       // This should be -'sd1

   parameter local_lt0 = local1 < 0;      // This should be 'd1
   parameter local_le0 = local1 <= 0;     // This should be 'd1
   parameter local_gt0 = local1 > 0;      // This should be 'd0
   parameter local_ge0 = local1 >= 0;     // This should be 'd0
   parameter local_0lt = 0 < local1;      // This should be 'd1
   parameter local_0le = 0 <= local1;     // This should be 'd1
   parameter local_0gt = 0 > local1;      // This should be 'd0
   parameter local_0ge = 0 >= local1;     // This should be 'd0

   reg err;
   initial begin
      err = 0;
      if (!local_lt0) err = 1;
      if (!local_le0) err = 1;
      if ( local_gt0) err = 1;
      if ( local_ge0) err = 1;
      if ( local_0lt) err = 1;
      if ( local_0le) err = 1;
      if (!local_0gt) err = 1;
      if (!local_0ge) err = 1;

      if (err == 0) $display("PASSED");
      else $display("FAILED");
   end
endmodule
