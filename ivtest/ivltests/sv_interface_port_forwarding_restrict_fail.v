// This tests that forwarding an interface formal cannot widen access
// beyond the parent-facing modport restriction.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

module test;
   bus_if bus();
   parent dut(.bus(bus));
endmodule

module parent(
   bus_if.consumer bus
);
   child u_child(.bus(bus));
endmodule

module child(
   bus_if bus
);
   logic sample;

   assign sample = bus.hidden;
endmodule

interface bus_if ();
   logic value;
   logic hidden;

   modport consumer(input value);
endinterface
