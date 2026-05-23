// This protects ordinary partial ANSI port parsing near interface
// formal grammar.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

module test;
   logic [3:0] value;
   logic [3:0] result;

   plain dut(value, result);

   initial begin
      value = 4'ha;
      #1;
      if (result !== 4'ha) begin
         $display("FAILED");
         $finish;
      end
      $display("PASSED");
   end
endmodule

module plain(
   input logic [3:0] value,
   output logic [3:0] result
);
   assign result = value;
endmodule
