`timescale 1ns/1ns
module main;

   submodule test();

endmodule // main

`timescale 10ns/1ns
module submodule;

   reg [63:0] val;
   initial begin
      #1 $display("$time = %0d", $time);
      val = $time;
      if (val !== 64'd1) begin
	 $display("FAILED -- value is %0d (should be 1)", val);
	 $finish;
      end
      $display("PASSED");
   end


endmodule // submodule
