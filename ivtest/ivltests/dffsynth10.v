module top();

reg       CLK;
reg [3:0] D;
reg       EN;
reg [3:0] Q;

always @(posedge CLK) begin
  if (EN) begin
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
  EN = 0;
  D = 4'b0000;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 4'bxxxx) failed = 1;
  EN = 1;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 4'b010x) failed = 1;
  EN = 0;
  D = 4'b1111;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 4'b010x) failed = 1;
  EN = 1;
  #1 CLK = 1;
  #1 CLK = 0;
  if (Q !== 4'b101x) failed = 1;
  #1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
