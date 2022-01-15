// Trigger breakage of Icarus Verilog CVS 2004-06-18
// $ iverilog netnet.v
// netnet.v:7: internal error: pin(3) out of bounds(3)
// netnet.v:7:               : typeid=6NetNet
// ivl: netlist.cc:208: Link &NetObj::pin (unsigned int): Assertion `idx < npins_' failed.
// $
// Larry Doolittle <ldoolitt@recycle.lbl.gov>

`timescale 1ns / 1ns

module netnet();

reg [2:0] s;
wire s_ones;
assign s_ones = (s==7);

initial begin
   s = 3'b111;

   #1 if (s_ones !== 1) begin
      $display("FAILED -- %b==7 returns %b", s, s_ones);
      $finish;
   end
   s = 3'b011;

   #1 if (s_ones !== 0) begin
      $display("FAILED -- %b==7 returns %b", s, s_ones);
      $finish;
   end

   $display("PASSED");
end
endmodule
