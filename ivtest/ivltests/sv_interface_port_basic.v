// This tests a SystemVerilog interface-typed module port with an
// explicit modport and a named actual interface instance connection.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

module test;
   logic [7:0] lhs;
   logic [7:0] rhs;

   bus_if bus();

   add_if dut(.bus(bus));

   assign bus.lhs = lhs;
   assign bus.rhs = rhs;

   initial begin
      lhs = 8'd5;
      rhs = 8'd7;
      #1;
      if (bus.sum !== 9'd12) begin
         $display("FAILED");
         $finish;
      end
      $display("PASSED");
   end
endmodule

interface bus_if #(parameter WIDTH = 8) ();
   logic [WIDTH-1:0] lhs;
   logic [WIDTH-1:0] rhs;
   logic [WIDTH:0]   sum;

   modport consumer(
      input  lhs,
      input  rhs,
      output sum
   );

endinterface

module add_if #(parameter WIDTH = 8) (
   bus_if.consumer bus
);
   assign bus.sum = bus.lhs + bus.rhs;
endmodule
