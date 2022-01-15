// Regression test for GitHub issue 9 : Efficiency of verinum and vpp_net
// pow() functions.

module bug();

reg  [5:0] ra;
reg  [5:0] ry;

wire [5:0] wa = 3;
wire [5:0] wy = wa ** 123456789;

localparam [5:0] pa = 3;
localparam [5:0] py = pa ** 123456789;

reg failed = 0;

initial begin
  #0;

  ra = 3;
  ry = ra ** 123456789;

  $display("%b", ry);
  if (ry !== 6'b110011) failed = 1;

  $display("%b", wy);
  if (wy !== 6'b110011) failed = 1;

  $display("%b", py);
  if (py !== 6'b110011) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
