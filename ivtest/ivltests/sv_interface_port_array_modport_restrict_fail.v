// This tests that modport restrictions apply through indexed interface
// formal array elements.

interface bus_if ();
   logic value;
   modport consumer(input value);
endinterface

module bad(bus_if.consumer bus[1]);
   assign bus[0].value = 1'b1;
endmodule

module test;
   bus_if buses[1]();
   bad dut(.bus(buses));
endmodule
