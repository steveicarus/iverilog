module genvar_scopes();

// This test is designed to check that genvars can be declared
// within a generate block and do not collide with genvars of
// the same name declared in the enclosing scope.

genvar  i;
genvar  j;

reg  [1:0] a[1:0];
wire [1:0] b[1:0];
wire [1:0] c[1:0];
wire [1:0] d[1:0];

for (i = 0; i < 2; i = i + 1) begin
  for (j = 0; j < 2; j = j + 1) begin
    assign b[i][j] = a[i][j];
  end
end

for (i = 0; i < 2; i = i + 1) begin
  genvar j;
  for (j = 0; j < 2; j = j + 1) begin
    assign c[i][j] = a[i][j];
  end
end

for (j = 0; j < 2; j = j + 1) begin
  genvar k;
  for (k = 0; k < 2; k = k + 1) begin
    assign d[j][k] = a[j][k];
  end
end

initial begin
  a[0] = 2'b01;
  a[1] = 2'b10;
  #1;
  $display("%b %b", b[0], b[1]);
  $display("%b %b", c[0], c[1]);
  $display("%b %b", d[0], d[1]);
  if ((b[0] === 2'b01) && (b[1] === 2'b10)
   && (c[0] === 2'b01) && (c[1] === 2'b10)
   && (d[0] === 2'b01) && (d[1] === 2'b10))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
