/*
 * This test deminstrates a complication in the handling of a vector
 * as a unit, instead of breaking it out. The problem is with the
 * legal inout expression that takes one bit of the vector. That
 * leads to a driver to the input of a part select, and also the
 * other way around. Yikes.
 */
module main;

   reg [1:0]  drv = 2'b0z;
   wire [1:0] a = drv;
   reg	      en;

   bi dut0(a[0], en);

   initial begin
      en <= 0;

      #1 $display("drv=%b en=%b, a=%b (should be 0z)", drv, en, a);
      if (a !== 2'b0z) begin
	 $display("FAILED");
	 $finish;
      end

      en <= 1;

      #1 $display("drv=%b en=%b, a=%b (should be 01)", drv, en, a);
      if (a !== 2'b01) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main

module bi (inout a, input en);

   reg val = 1;
   assign a = en? val : 1'bz;

endmodule // bi
