module test();

integer array1[0:3];
integer array2[1:4];
integer array3[0:3];
integer array4[1:4];

integer i;

reg failed;

for (genvar i = 0; i < 4; ++i) begin
  initial array1[i] = i;
end

for (genvar i = 4; i > 0; --i) begin
  initial array2[i] = i;
end

for (genvar i = 0; i < 4; i++) begin
  initial array3[i] = i;
end

for (genvar i = 4; i > 0; i--) begin
  initial array4[i] = i;
end

initial begin
  #1 failed = 0;

  for (i = 0; i < 4; ++i) begin
    $display(array1[i]);
    if (array1[i] !== i) failed = 1;
  end

  for (i = 1; i < 5; ++i) begin
    $display(array2[i]);
    if (array2[i] !== i) failed = 1;
  end

  for (i = 0; i < 4; ++i) begin
    $display(array3[i]);
    if (array3[i] !== i) failed = 1;
  end

  for (i = 1; i < 5; ++i) begin
    $display(array4[i]);
    if (array4[i] !== i) failed = 1;
  end

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
