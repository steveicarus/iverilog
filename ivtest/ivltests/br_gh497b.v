// The IEEE standard allows the out-of-bounds part-selects to be flagged as
// compile-time errors. If they are not, this test should pass.
module top;

wire [3:0][3:0] array1;
wire [3:0][3:0] array2;
wire [3:0][3:0] array3;
wire [3:0][3:0] array4;
wire [3:0][3:0] array5;
wire [3:0][3:0] array6;

assign array1[-2+:2] = 8'h00;
assign array1[ 0+:2] = 8'h21;
assign array1[ 2+:2] = 8'h43;
assign array1[ 4+:2] = 8'h55;

assign array1[-1+:2] = 8'h10;
assign array2[ 1+:2] = 8'h32;
assign array2[ 3+:2] = 8'h54;

assign array3[-1-:2] = 8'h00;
assign array3[ 1-:2] = 8'h21;
assign array3[ 3-:2] = 8'h43;
assign array3[ 5-:2] = 8'h55;

assign array4[ 0-:2] = 8'h10;
assign array4[ 2-:2] = 8'h32;
assign array4[ 4-:2] = 8'h54;

assign array5[-1:-2] = 8'h00;
assign array5[ 1:0 ] = 8'h21;
assign array5[ 3:2 ] = 8'h43;
assign array5[ 5:4 ] = 8'h55;

assign array6[ 0:-1] = 8'h10;
assign array6[ 2:1 ] = 8'h32;
assign array6[ 4:3 ] = 8'h54;

reg failed = 0;

initial begin
  $display("%h", array1);
  if (array1 !== 16'h4321) failed = 1;
  $display("%h", array2);
  if (array2 !== 16'h4321) failed = 1;
  $display("%h", array3);
  if (array3 !== 16'h4321) failed = 1;
  $display("%h", array4);
  if (array4 !== 16'h4321) failed = 1;
  $display("%h", array5);
  if (array5 !== 16'h4321) failed = 1;
  $display("%h", array6);
  if (array6 !== 16'h4321) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
