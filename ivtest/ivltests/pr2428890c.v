module top;
  lower #(1, 2, 3) dut();
endmodule

module lower;
   parameter one = 1;                  // This should be 'sd1
   parameter two = 2;                  // This should be 'sd2
   parameter three = 0;                // This should be 'sd3
   parameter local1 = one - two;       // This should be -'sd1

   parameter local_t1 = local1 * 1;     // This should be -'d1
   parameter local_d1 = local1 / 1;     // This should be -'d1

   reg err;
   initial begin
      err = 0;
      if (local_t1 !== -1) err = 1;
      if (local_t1 > 0)    err = 1;
      if (local_d1 !== -1) err = 1;
      if (local_d1 > 0)    err = 1;

      if (! $is_signed(local_t1)) err = 1;
      if (! $is_signed(local_d1)) err = 1;
      if (! $is_signed(local1))   err = 1;

      if (err == 0) $display("PASSED");
      else $display("FAILED");
   end
endmodule
