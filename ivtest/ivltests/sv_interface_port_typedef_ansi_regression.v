// This protects typedef-based ANSI ports from being interpreted as
// interface-typed formals.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

typedef logic [3:0] nibble_t;

module test;
   nibble_t value;
   nibble_t result;

   typed dut(value, result);

   initial begin
      value = 4'h6;
      #1;
      if (result !== 4'h6) begin
         $display("FAILED");
         $finish;
      end
      $display("PASSED");
   end
endmodule

module typed(
   input nibble_t value,
   output nibble_t result
);
   assign result = value;
endmodule
