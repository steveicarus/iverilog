// This tests the diagnostic path for an interface-typed module port
// that names a modport missing from the interface definition.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

module test;
   bus_if bus();
   bus_user dut(.bus(bus));
endmodule

interface bus_if ();
   logic value;

   modport producer(output value);
endinterface

module bus_user(
   bus_if.consumer bus
);
   initial $display("FAILED");
endmodule
