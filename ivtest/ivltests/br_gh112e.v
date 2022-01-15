module bug();

reg [0:-1][14:-1][6:-1] array;

reg failed = 0;

integer i;

reg signed [4:0] index;

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
    array[-1][-5'sd1+index] = {4'd0, index[3:0]};
    array[ 0][-5'sd1+index] = {4'd1, index[3:0]};
  end
  $display("%h", array);
  if (array !== 256'h1f1e1d1c1b1a191817161514131211100f0e0d0c0b0a09080706050403020100)
    failed = 1;
  for (i = 0; i < 16; i++) begin
    index = i[3:0];
    $display("%h : %h %h", index, array[-1][-5'sd1+index], array[0][-5'sd1+index]);
    if (array[-1][-5'sd1+index] !== {4'd0, index[3:0]}) failed = 1;
    if (array[ 0][-5'sd1+index] !== {4'd1, index[3:0]}) failed = 1;
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
