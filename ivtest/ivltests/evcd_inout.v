/*
 * Extended-VCD ($dumpports) inout conflict-state test. The DUT and the
 * testbench each drive the shared inout `io` through a conditional (tri)
 * driver, so the resolver retains both sides and the writer emits the IEEE
 * 1364-2005 18.4.3 state characters: F (neither drives), H (module drives,
 * external hi-Z), U (external drives, module hi-Z), A (in 0 / out 1),
 * B (in 1 / out 0) and 1 (both drive 1).
 */
module dut(inout wire io, input wire oe, input wire ov);
   assign io = oe ? ov : 1'bz;
endmodule

module test;
   reg  oe, ov;   // control the module-side (output) drive
   reg  te, tv;   // control the external (testbench) drive
   wire io;

   assign io = te ? tv : 1'bz;

   dut u(.io(io), .oe(oe), .ov(ov));

   initial begin
      $dumpports(u, "work/evcd_inout.evcd");
      oe = 0; ov = 0; te = 0; tv = 0; #5;   // F  : neither side drives
      oe = 1; ov = 1; te = 0; tv = 0; #5;   // H  : module 1, external hi-Z
      oe = 0; ov = 0; te = 1; tv = 1; #5;   // U  : external 1, module hi-Z
      oe = 1; ov = 1; te = 1; tv = 0; #5;   // A  : external 0, module 1
      oe = 1; ov = 0; te = 1; tv = 1; #5;   // B  : external 1, module 0
      oe = 1; ov = 1; te = 1; tv = 1; #5;   // 1  : both drive 1
      $finish;
   end
endmodule
