// This tests rejection of a whole interface array actual whose size
// does not match the formal interface array size.

interface bus_if ();
endinterface

module child(bus_if bus[2]);
endmodule

module test;
   bus_if buses[1]();
   child dut(.bus(buses));
endmodule
