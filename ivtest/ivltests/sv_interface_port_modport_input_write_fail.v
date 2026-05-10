// This tests rejection of an assignment through a member declared input
// by the selected interface modport.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

module test;
   bus_if bus();
   bus_user dut(.bus(bus));
endmodule

interface bus_if ();
   logic value;

   modport consumer(input value);
endinterface

module bus_user(
   bus_if.consumer bus
);
   assign bus.value = 1'b1;
endmodule
