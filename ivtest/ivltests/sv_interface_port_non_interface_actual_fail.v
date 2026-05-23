// This tests rejection of a non-interface actual connected to an
// interface-typed module port.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

module test;
   logic value;
   bus_user dut(.bus(value));
endmodule

interface bus_if ();
   logic value;

   modport consumer(input value);
endinterface

module bus_user(
   bus_if.consumer bus
);
   initial $display("FAILED");
endmodule
