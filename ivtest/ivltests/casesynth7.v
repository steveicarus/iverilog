// Incomplete case statements in asynchronous logic are dangerous in
// synthesisable code, as in real hardware the inferred latch will be
// sensitive to glitches as the case select value changes. Check that
// the compiler outputs a warning for this.
module mux(

input wire [2:0] sel,
input wire [2:0] i1,
input wire [2:0] i2,
input wire [2:0] i3,
input wire [2:0] i4,
output reg [2:0] o

);

(* ivl_synthesis_on *)
always @* begin
  case (sel)
    0 : o = 0;
    1 : o = i1;
    2 : o = i2;
    3 : o = i3;
    4 : o = i4;
  endcase
end
(* ivl_synthesis_off *)

initial $display("PASSED");

endmodule
