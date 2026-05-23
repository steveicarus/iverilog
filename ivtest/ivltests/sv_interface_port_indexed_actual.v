// This tests connecting a scalar interface-typed formal to one element
// of an interface instance array.

interface bus_if ();
   logic [7:0] value;
   modport producer(output value);
   modport consumer(input value);
endinterface

module drive(input [7:0] val, bus_if.producer bus);
   assign bus.value = val;
endmodule

module sample(output [7:0] y, bus_if.consumer bus);
   assign y = bus.value;
endmodule

module test;
   bus_if buses[2]();
   wire [7:0] y;

   drive d0(8'd37, buses[0]);
   sample s0(.bus(buses[0]), .y(y));

   initial begin
      #1;
      if (y !== 8'd37) begin
	 $display("FAILED: y=%0d", y);
	 $finish;
      end
      $display("PASSED");
   end
endmodule
