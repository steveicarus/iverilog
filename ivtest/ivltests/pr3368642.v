module bug;

reg  [3:0] r1;
wire [3:0] w1;
wire [3:0] w2;

assign w1 = r1;
assign w2 = w1;

reg fail = 0;

initial begin
  r1 = 0;
  #1 $display("%b %b %b", r1, w1, w2);
  if ((r1 !== 4'b0000) || (w1 !== 4'b0000) || (w2 !== 4'b0000)) fail = 1;
  force w1 = 4'bz;
  #1 $display("%b %b %b", r1, w1, w2);
  if ((r1 !== 4'b0000) || (w1 !== 4'bzzzz) || (w2 !== 4'bzzzz)) fail = 1;
  r1 = 1;
  #1 $display("%b %b %b", r1, w1, w2);
  if ((r1 !== 4'b0001) || (w1 !== 4'bzzzz) || (w2 !== 4'bzzzz)) fail = 1;
  release w1;
  #1 $display("%b %b %b", r1, w1, w2);
  if ((r1 !== 4'b0001) || (w1 !== 4'b0001) || (w2 !== 4'b0001)) fail = 1;
  r1 = 2;
  #1 $display("%b %b %b", r1, w1, w2);
  if ((r1 !== 4'b0010) || (w1 !== 4'b0010) || (w2 !== 4'b0010)) fail = 1;

  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
