// This tests rejection of access to an interface member that is not
// listed in the selected modport.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

module test;
   bus_if bus();
   bus_user dut(.bus(bus));
endmodule

interface bus_if ();
   logic visible;
   logic hidden;

   modport consumer(input visible);
endinterface

module bus_user(
   bus_if.consumer bus
);
   logic sample;

   assign sample = bus.hidden;
endmodule
