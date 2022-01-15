module bug();

reg        d;
reg [31:0] x;
reg [31:0] y;
reg [31:0] z;

initial begin
  d = 1;
  x = 32'hffffffff << {d, 64'd0};
  y = 32'hffffffff >> {d, 64'd0};
  z = 32'hffffffff >>> {d, 64'd0};
  $display("%h", x);
  $display("%h", y);
  $display("%h", z);
  if (x === 32'd0 && y === 32'd0 && z === 32'd0)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
