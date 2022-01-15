module top;

reg [3:0][3:0] array1;
reg [3:0][3:0] array2;
reg [3:0][3:0] array3;
reg [3:0][3:0] array4;
reg [3:0][3:0] array5;
reg [3:0][3:0] array6;

reg failed = 0;

initial begin
  array1[0+:2] = 8'h21;
  array1[2+:2] = 8'h43;

  array2[1+:2] = 8'h32;

  array3[1-:2] = 8'h21;
  array3[3-:2] = 8'h43;

  array4[2-:2] = 8'h32;

  array5[1:0] = 8'h21;
  array5[3:2] = 8'h43;

  array6[2:1] = 8'h32;

  $display("%h", array1);
  if (array1 !== 16'h4321) failed = 1;
  $display("%h", array2);
  if (array2 !== 16'hx32x) failed = 1;
  $display("%h", array3);
  if (array3 !== 16'h4321) failed = 1;
  $display("%h", array4);
  if (array4 !== 16'hx32x) failed = 1;
  $display("%h", array5);
  if (array5 !== 16'h4321) failed = 1;
  $display("%h", array6);
  if (array6 !== 16'hx32x) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
