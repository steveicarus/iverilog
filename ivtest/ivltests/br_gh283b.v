module bug();

reg [64:0] d;
reg [31:0] x;
reg [31:0] y;
reg [31:0] z;

initial begin
  d = 65'h1_0000_0000_0000_0000;
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
