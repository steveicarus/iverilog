module top();

reg       CLK;
reg [3:0] D;
reg       EN1;
reg       EN2;
reg       EN3;
reg [3:0] Q;

always @(posedge CLK) begin
  if (EN1) Q[1] <= D[1];
  if (EN2) Q[2] <= D[2];
  if (EN3) Q[3] <= D[3];
end

reg failed;

(* ivl_synthesis_off *)
initial begin
  failed = 0;
  $monitor("%b %b %b %b %b %b", CLK, EN1, EN2, EN3, D, Q);
  CLK = 0;
  EN1 = 0;
  EN2 = 0;
  EN3 = 0;
  D = 4'b0000;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 4'bxxxx) failed = 1;
  EN1 = 1;
  D = 4'b0000;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 4'bxx0x) failed = 1;
  EN1 = 0;
  EN2 = 1;
  D = 4'b1111;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 4'bx10x) failed = 1;
  EN2 = 0;
  EN3 = 1;
  D = 4'b0000;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 4'b010x) failed = 1;
  #1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
