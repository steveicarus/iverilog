// Extended VCD ($dumpports): vector (bus) input / output / inout ports.
module evcd_bus_dut (input wire [3:0] din, output wire [3:0] dout,
                     inout wire [3:0] bus);
   assign dout = ~din;
   assign bus  = din[0] ? din : 4'bzzzz;
endmodule

module evcd_bus;
   reg  [3:0] din;
   wire [3:0] dout, bus;
   evcd_bus_dut u (.din(din), .dout(dout), .bus(bus));
   initial begin
      $dumpports(u, "work/evcd_bus.evcd");
      din = 4'b0000; #5;
      din = 4'b1010; #5;
      din = 4'b1111; #5;
      $finish;
   end
endmodule
