// This tests rejection of an unconnected positional interface port.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

module test;
   bus_user dut();
endmodule

module bus_user(
   bus_if.consumer bus
);
endmodule

interface bus_if ();
   logic value;

   modport consumer(input value);
endinterface
