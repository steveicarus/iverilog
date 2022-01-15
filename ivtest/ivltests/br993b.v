module br993a();

reg       clk;
reg       a;
reg       b;
reg [1:0] q;

(* ivl_synthesis_on *)
always @(posedge clk) begin
  if (a) q <= 1;
  if (b) q <= 2;
end
(* ivl_synthesis_off *)

reg failed;
initial begin
  clk = 0;

  a = 1;
  b = 1;
  #1 clk = 1;
  #1 clk = 0;
  $display("%d", q);
  if (q !== 2'd2) failed = 1;

  a = 0;
  b = 0;
  #1 clk = 1;
  #1 clk = 0;
  $display("%d", q);
  if (q !== 2'd2) failed = 1;

  a = 1;
  b = 0;
  #1 clk = 1;
  #1 clk = 0;
  $display("%d", q);
  if (q !== 2'd1) failed = 1;

  a = 0;
  b = 0;
  #1 clk = 1;
  #1 clk = 0;
  $display("%d", q);
  if (q !== 2'd1) failed = 1;

  a = 0;
  b = 1;
  #1 clk = 1;
  #1 clk = 0;
  $display("%d", q);
  if (q !== 2'd2) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
