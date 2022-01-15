module test();

reg [3:0] array[0:1][0:2];

reg [3:0] expected;

reg failed = 0;

initial begin
  for (int i = 0; i < 2; i++) begin
    for (int j = 0; j < 3; j++) begin
      array[i][j] = i * 4 + j;
    end
  end
  foreach (array[i,j,k]) begin
    expected = i * 4 + j;
    $display("Value of array[%0d][%0d][%0d]=%b", i, j, k, array[i][j][k]);
    if (array[i][j][k] !== expected[k]) failed = 1;
  end
  foreach (array[i,j]) begin
    expected = i * 4 + j;
    $display("Value of array[%0d][%0d]=%h", i, j, array[i][j]);
    if (array[i][j] !== expected) failed = 1;
  end
  foreach (array[i]) begin
    expected = i * 4;
    $display("Value of array[%0d][0]=%h", i, array[i][0]);
    if (array[i][0] !== expected) failed = 1;
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
