module bug();

reg [31:0] d;
reg [31:0] x;
reg [31:0] y;
reg [31:0] z;

initial begin
  d = 32'hffff0000;
  x = 32'hffffffff << d;
  y = 32'hffffffff >> d;
  z = 32'hffffffff >>> d;
  $display("%h", x);
  $display("%h", y);
  $display("%h", z);
  if (x === 32'd0 && y === 32'd0 && z === 32'd0)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
