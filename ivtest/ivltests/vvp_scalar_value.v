module vvp_scalar_value();

reg  [2:0] v1;
reg  [2:0] v2;

wire [2:0] w1;
wire [2:0] w2;
wire [2:0] w3;

assign ( highz1, strong0) w1 = v1;
assign (strong1,  highz0) w2 = v1;

assign ( highz1, strong0) w3 = v1;
assign (strong1,  highz0) w3 = v2;

reg failed;

initial begin
  failed = 0;

  v1 = 3'bz10; #1;
  $display("%b %v %v %v", w1, w1[2], w1[1], w1[0]);
  if (w1 !== 3'bzz0) failed = 1;
  $display("%b %v %v %v", w2, w2[2], w2[1], w2[0]);
  if (w2 !== 3'bz1z) failed = 1;

  v2 = 3'b000; #1;
  $display("%b %v %v %v", w3, w3[2], w3[1], w3[0]);
  if (w3 !== 3'bzz0) failed = 1;

  v2 = 3'b111; #1;
  $display("%b %v %v %v", w3, w3[2], w3[1], w3[0]);
  if (w3 !== 3'b11x) failed = 1;

  v2 = 3'bzzz; #1;
  $display("%b %v %v %v", w3, w3[2], w3[1], w3[0]);
  if (w3 !== 3'bzz0) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
