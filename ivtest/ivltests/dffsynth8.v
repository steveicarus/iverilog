module dff();

reg clk;
reg rst;
reg ce;
reg [3:0] s;
reg [3:0] d;
reg [3:0] q;

always @(negedge clk or posedge rst) begin
  if (rst)
    q <= s;
  else if (ce)
    q <= d;
end

(* ivl_synthesis_off *)
reg failed = 0;

initial begin
  $monitor("%b %b %b %b", rst, clk, d, q);
  clk = 1'b0;
  ce  = 1'b0;
  rst = 1'b0;
  s = 4'b1001;
  d = 4'b0110;
  #1;
  if (q !== 4'bxxxx) failed = 1;
  rst = 1'b1;
  #1;
  if (q !== 4'b1001) failed = 1;
  clk = 1'b1;
  #1;
  if (q !== 4'b1001) failed = 1;
  clk = 1'b0;
  #1;
  if (q !== 4'b1001) failed = 1;
  rst = 1'b0;
  #1;
  if (q !== 4'b1001) failed = 1;
  clk = 1'b1;
  #1;
  if (q !== 4'b1001) failed = 1;
  clk = 1'b0;
  #1;
  if (q !== 4'b1001) failed = 1;
  ce  = 1'b1;
  #1;
  if (q !== 4'b1001) failed = 1;
  clk = 1'b1;
  #1;
  if (q !== 4'b1001) failed = 1;
  clk = 1'b0;
  #1;
  if (q !== 4'b0110) failed = 1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule // dff
