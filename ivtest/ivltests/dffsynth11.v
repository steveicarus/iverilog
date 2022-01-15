module top();

reg       CLK;
reg       RST;
reg [3:1] D;
reg       EN;
reg [3:1] Q;

always @(posedge CLK or posedge RST) begin
  if (RST) begin
    Q[1] <= 1'b0;
    Q[2] <= 1'b1;
    Q[3] <= 1'b0;
  end
  else if (EN) begin
    Q[1] <=  D[1];
    Q[2] <= ~D[2];
    Q[3] <=  D[3];
  end
end

reg failed;

(* ivl_synthesis_off *)
initial begin
  failed = 0;
  $monitor("%b %b %b %b", CLK, EN, D, Q);
  CLK = 0;
  RST = 1;
  EN = 0;
  D = 3'b111;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 3'b010) failed = 1;
  EN = 1;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 3'b010) failed = 1;
  RST = 0;
  EN = 0;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 3'b010) failed = 1;
  EN = 1;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 3'b101) failed = 1;
  #1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
