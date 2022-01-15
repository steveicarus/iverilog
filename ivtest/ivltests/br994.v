module test();

reg       clk;
reg       sel;
reg [7:0] a;
reg [6:0] b;
reg [5:0] q;

(* ivl_synthesis_on *)
always @(posedge clk) begin
  if (sel)
    q <= b;
  else
    q <= a;
end
(* ivl_synthesis_off *)

reg failed;

initial begin
  a = 'haa;
  b = 'hbb;
  clk = 0;
  sel = 0;
  #1 clk = 1;
  #1 clk = 0;
  $display("%h", q);
  if (q !== 6'h2a) failed = 1;
  sel = 1;
  #1 clk = 1;
  #1 clk = 0;
  $display("%h", q);
  if (q !== 6'h3b) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
