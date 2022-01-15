module bug();

reg [1:0][15:0][7:0] array;

reg failed = 0;

integer i;

reg [3:0] index;

initial begin
  i = $bits(array);
  $display("width 0 = %0d", i);
  if (i !== 256) failed = 1;

  i = $bits(array[0]);
  $display("width 1 = %0d", i);
  if (i !== 128) failed = 1;

  i = $bits(array[0][0]);
  $display("width 2 = %0d", i);
  if (i !== 8) failed = 1;

  for (i = 0; i < 16; i++) begin
    index = i[3:0];
    array[0][index] = {4'd0, index};
    array[1][index] = {4'd1, index};
  end
  $display("%h", array);
  if (array !== 256'h1f1e1d1c1b1a191817161514131211100f0e0d0c0b0a09080706050403020100)
    failed = 1;
  for (i = 0; i < 16; i++) begin
    index = i[3:0];
    $display("%h : %h %h", index, array[0][index], array[1][index]);
    if (array[0][index] !== {4'd0, index}) failed = 1;
    if (array[1][index] !== {4'd1, index}) failed = 1;
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
