module test(_clock);
// --- params ---
input  _clock;
// --- vars ---
reg [15:0] m;
reg [15:0] a;
// --- body ---

always @ (posedge _clock)
begin
   m = 1;
   a[15*m] = 1; // accepted by icarus, but thought that verilog expected a const-expression
   a[15:0] = 1;  // ok
   a[15*m:0] = 1; // causes error, but not sure if this is legal syntax
end
endmodule // test
