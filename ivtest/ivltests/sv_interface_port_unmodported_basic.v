// This tests a concrete interface-typed module port without an explicit
// modport restriction.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

interface bus_if ();
   logic [7:0] lhs;
   logic [7:0] rhs;
   logic [8:0] sum;
endinterface

module test;
   logic [7:0] lhs;
   logic [7:0] rhs;

   bus_if bus();

   add_if dut(.bus(bus));

   assign bus.lhs = lhs;
   assign bus.rhs = rhs;

   initial begin
      lhs = 8'd9;
      rhs = 8'd4;
      #1;
      if (bus.sum !== 9'd13) begin
         $display("FAILED");
         $finish;
      end
      $display("PASSED");
   end
endmodule

module add_if(
   bus_if bus
);
   assign bus.sum = bus.lhs + bus.rhs;
endmodule
