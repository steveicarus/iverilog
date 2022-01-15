module mod #(
  parameter  A = 1,
  localparam B = A + 1,
  parameter  C = B - 1
) (
  input [A-1:0] a,
  input [B-1:0] b,
  input [C-1:0] c
);

endmodule

module top();

reg [3:0] a  = 'ha;
reg [4:0] b  = 'hb;
reg [5:0] c  = 'hc;

mod #(4, 6) m(a, b, c);

initial begin
  $display("%0d %0d %0d", m.A, m.B, m.C);
  $display("%0d %0d %0d", $bits(m.a), $bits(m.b), $bits(m.c));
  if (m.A === 4 && $bits(m.a) === 4
  &&  m.B === 5 && $bits(m.b) === 5
  &&  m.C === 6 && $bits(m.c) === 6)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
