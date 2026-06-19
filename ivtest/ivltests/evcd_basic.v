// Extended VCD ($dumpports): scalar input / output / inout ports.
module evcd_basic_dut (input wire a, input wire b,
                       output wire y, inout wire io);
   assign y  = a & b;
   assign io = a ? b : 1'bz;
endmodule

module evcd_basic;
   reg  a, b;
   wire y, io;
   evcd_basic_dut u (.a(a), .b(b), .y(y), .io(io));
   initial begin
      $dumpports(u, "work/evcd_basic.evcd");
      a = 1'b0; b = 1'b0; #5;
      a = 1'b1; b = 1'b0; #5;
      a = 1'b1; b = 1'b1; #5;
      a = 1'b0; b = 1'b1; #5;
      $finish;
   end
endmodule
