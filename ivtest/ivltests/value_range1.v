// This program is about testing that the value ranges parse and work for
// integer parameters.
module test(input wire in);

   parameter integer foo = 0 from [-10 : 10]  exclude [1:2);
   parameter integer bar = 0 from (-inf:0];

   initial begin
      $display("foo = %d", foo);
      $display("PASSED");
      $finish;
   end

endmodule // test

module main;

   reg rrr = 0;
   test #(.foo(2), .bar(-5)) dut (rrr);

endmodule // main
