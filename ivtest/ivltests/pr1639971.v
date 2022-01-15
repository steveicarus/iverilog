module RegisterArrayBug01;

   reg [15:0] rf[0:7];
   wire [3:0] rf_0_slice0 = rf[0][3:0];

   wire [15:0] rf_0 = rf[0];

   initial begin
      $monitor($time,, "rf and slice: %h %h", rf_0, rf_0_slice0);

      rf[0] = 16'hffff;
      #10 rf[0] = 16'h0000;
      #10 rf[0] = 16'hbeef;
      #10 $finish(0);
   end

endmodule

/* Program fails to compile with result:
 elab_net.cc:1738: failed assertion `msb_ == 0'
 */
