module mux(

input wire [1:0] sel,
input wire [2:0] i0,
input wire [2:0] i1,
input wire [2:0] i2,
input wire [2:0] i3,
input wire [2:0] i4,
output reg [2:0] o

);

always @* begin
  case (sel)
    0 : o = i0;
    1 : o = i1;
    2 : o = i2;
    3 : o = i3;
    2 : o = i4;
  endcase
end

endmodule

module test();

reg  [1:0] sel;
wire [2:0] out;

mux mux(sel, 3'd0, 3'd1, 3'd2, 3'd3, 3'd4, out);

reg failed;

(* ivl_synthesis_off *)
initial begin
  failed = 0;
  sel = 0;
  repeat (4) begin
    #1 $display("%d : %b", sel, out);
    if (out !== sel) failed = 1;
    sel = sel + 1;
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
