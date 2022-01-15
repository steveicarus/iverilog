/* Crash.v - reproduces a simulation-time crash (PLI assertion failure)

Copyright: Bluespec, Inc. 2010
License:   GPLv2 or later

Transcript:
> uname -a
Linux jnewbern-laptop 2.6.32-21-generic #32-Ubuntu SMP Fri Apr 16 08:09:38 UTC 2010 x86_64 GNU/Linux
> iverilog -V
Icarus Verilog version 0.9.2  (v0_9_2)
 ...
> iverilog -v -o crash -Wall Crash.v
...
/usr/lib/ivl/system.sft: Processing System Function Table file.
/usr/lib/ivl/v2005_math.sft: Processing System Function Table file.
/usr/lib/ivl/va_math.sft: Processing System Function Table file.
Using language generation: IEEE1364-2005,no-specify,xtypes,icarus-misc
PARSING INPUT
LOCATING TOP-LEVEL MODULES
   Crash
 ... done, 0 seconds.
ELABORATING DESIGN
 ... done, 0 seconds.
RUNNING FUNCTORS
 -F cprop ...
 -F nodangle ...
 ... 1 iterations deleted 0 dangling signals and 0 events.
 ... 2 iterations deleted 0 dangling signals and 1 events.
CALCULATING ISLANDS
 ... done, 0 seconds.
CODE GENERATION
 ... invoking target_design
 ... done, 0 seconds.
STATISTICS
lex_string: add_count=50 hit_count=17
> ./crash
VCD info: dumpfile dump.vcd opened for output.
VCD warning: $dumpvars ignored, previously called at simtime 0
vvp: vpi_priv.cc:165: PLI_INT32 vpi_free_object(__vpiHandle*): Assertion `ref' failed.
Aborted
*/
module Crash();

   // Create clock
   reg CLK;

   initial begin
      CLK = 1'b0;
   end

   always begin
      #5;
      CLK = 1'b1;
      #5;
      CLK = 1'b0;
   end

   // Setup dumpfile at startup
   initial begin
      $dumpfile("dump.vcd");
      $dumpvars;
   end

   // Count cycles
   reg [7:0] counter;

   initial begin
      counter = 8'd0;
   end

   always @(posedge CLK) begin
      counter <= counter + 1;
   end

   // Call system tasks on particular cycles
   always@(posedge CLK)
   begin
      if (counter == 8'd2) $dumpvars; // repeated!
      if (counter >= 8'd200) begin
         $display("PASSED");
         $finish(32'd0);
      end
   end

endmodule
