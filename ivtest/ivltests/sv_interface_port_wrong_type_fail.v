// This tests rejection of an actual interface instance whose type does
// not match the interface type of the formal module port.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

module test;
   other_if bus();
   bus_user dut(.bus(bus));
endmodule

interface bus_if ();
   logic value;

   modport consumer(input value);
endinterface

interface other_if ();
   logic value;

   modport consumer(input value);
endinterface

module bus_user(
   bus_if.consumer bus
);
   initial $display("FAILED");
endmodule
