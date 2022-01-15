module test;

reg [7:0] array[7:0];

reg [2:0] index;

reg [7:0] value;

reg failed = 0;

initial begin
  array[0] = 1;
  index = 7;
  value = array[index + 1];
  $display("%h", value);
  if (value !== 8'bx) failed = 1;
  value = array[index + 3'd1];
  $display("%h", value);
  if (value !== 8'd1) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
