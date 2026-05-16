// This tests connecting scalar interface-typed formals to interface
// array elements selected by generated constant indices.

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
   wire [7:0] y[2];

   genvar i;
   generate
      for (i = 0; i < 2; i = i + 1) begin : gen
	 localparam [7:0] VAL = 8'd11 + i;
	 drive d(VAL, buses[i]);
	 sample s(.y(y[i]), .bus(buses[i]));
      end
   endgenerate

   initial begin
      #1;
      if (y[0] !== 8'd11) begin
	 $display("FAILED: y[0]=%0d", y[0]);
	 $finish;
      end
      if (y[1] !== 8'd12) begin
	 $display("FAILED: y[1]=%0d", y[1]);
	 $finish;
      end
      $display("PASSED");
   end
endmodule
