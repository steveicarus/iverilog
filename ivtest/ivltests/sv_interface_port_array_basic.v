// This tests a one-dimensional interface formal array connected to a
// whole interface instance array, then indexed inside the receiving
// module.

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

module child_array(output [7:0] y0, output [7:0] y1,
		   bus_if.consumer bus[2]);
   sample c0(.y(y0), .bus(bus[0]));
   sample c1(.y(y1), .bus(bus[1]));
endmodule

module test;
   bus_if buses[2]();
   wire [7:0] y0;
   wire [7:0] y1;

   drive d0(8'd21, buses[0]);
   drive d1(8'd42, buses[1]);
   child_array dut(.bus(buses), .y0(y0), .y1(y1));

   initial begin
      #1;
      if (y0 !== 8'd21) begin
	 $display("FAILED: y0=%0d", y0);
	 $finish;
      end
      if (y1 !== 8'd42) begin
	 $display("FAILED: y1=%0d", y1);
	 $finish;
      end
      $display("PASSED");
   end
endmodule
