// This tests positional binding of an interface-typed module port.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

interface bus_if ();
   logic value;
   logic sample;

   modport consumer(input value, output sample);
endinterface

module test;
   bus_if bus();

   assign bus.value = 1'b1;
   bus_user dut(bus);

   initial begin
      #1;
      if (bus.sample !== 1'b1) begin
         $display("FAILED");
         $finish;
      end
      $display("PASSED");
   end
endmodule

module bus_user(
   bus_if.consumer bus
);
   assign bus.sample = bus.value;
endmodule
