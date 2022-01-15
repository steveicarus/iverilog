// We don't (currently) support a case statement where both the case select
// and one or more case items are variables in asynchronous logic synthesis.
// Check the compiler handles and rejects this code.
module mux(

input wire [2:0] sel,
input wire [2:0] i1,
input wire [2:0] i2,
input wire [2:0] i3,
input wire [2:0] i4,
input wire [2:0] i5,
output reg [2:0] o

);

(* ivl_synthesis_on *)
always @* begin
  case (sel)
    0 : o = 0;
    1 : o = i1;
    2 : o = i2;
    3 : o = i3;
   i5 : o = i4;
   default:
     o = 3'bx;
  endcase
end
(* ivl_synthesis_off *)

initial $display("PASSED");

endmodule
