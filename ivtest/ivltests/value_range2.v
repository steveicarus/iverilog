// This program is about testing that the value ranges parse and work for
// integer parameters.
module test(input wire in);

   parameter real foo = 0.0 from [-10.0 : 10.0]  exclude [1:2);
   parameter real bar = 0 from (-inf:0];

   initial begin
      $display("foo = %f", foo);
      $display("PASSED");
      $finish;
   end

endmodule // test

module main;

   reg rrr = 0;
   test #(.foo(2), .bar(-5.0)) dut (rrr);

endmodule // main
