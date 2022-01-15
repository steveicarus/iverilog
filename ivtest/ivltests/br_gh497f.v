// The IEEE standard allows the out-of-bounds part-selects to be flagged as
// compile-time errors. If they are not, this test should pass.
module top;

reg [3:0][3:0] array;

reg failed = 0;

initial begin
  array = 16'h4321;

  $display("%h", array[-2+:2]); if (array[-2+:2] !== 8'hxx) failed = 1;
  $display("%h", array[-1+:2]); if (array[-1+:2] !== 8'h1x) failed = 1;
  $display("%h", array[ 0+:2]); if (array[ 0+:2] !== 8'h21) failed = 1;
  $display("%h", array[ 1+:2]); if (array[ 1+:2] !== 8'h32) failed = 1;
  $display("%h", array[ 2+:2]); if (array[ 2+:2] !== 8'h43) failed = 1;
  $display("%h", array[ 3+:2]); if (array[ 3+:2] !== 8'hx4) failed = 1;
  $display("%h", array[ 4+:2]); if (array[ 4+:2] !== 8'hxx) failed = 1;

  $display("%h", array[-1-:2]); if (array[-1-:2] !== 8'hxx) failed = 1;
  $display("%h", array[ 0-:2]); if (array[ 0-:2] !== 8'h1x) failed = 1;
  $display("%h", array[ 1-:2]); if (array[ 1-:2] !== 8'h21) failed = 1;
  $display("%h", array[ 2-:2]); if (array[ 2-:2] !== 8'h32) failed = 1;
  $display("%h", array[ 3-:2]); if (array[ 3-:2] !== 8'h43) failed = 1;
  $display("%h", array[ 4-:2]); if (array[ 4-:2] !== 8'hx4) failed = 1;
  $display("%h", array[ 5-:2]); if (array[ 5-:2] !== 8'hxx) failed = 1;

  $display("%h", array[-1:-2]); if (array[-1:-2] !== 8'hxx) failed = 1;
  $display("%h", array[ 0:-1]); if (array[ 0:-1] !== 8'h1x) failed = 1;
  $display("%h", array[ 1:0 ]); if (array[ 1:0 ] !== 8'h21) failed = 1;
  $display("%h", array[ 2:1 ]); if (array[ 2:1 ] !== 8'h32) failed = 1;
  $display("%h", array[ 3:2 ]); if (array[ 3:2 ] !== 8'h43) failed = 1;
  $display("%h", array[ 4:3 ]); if (array[ 4:3 ] !== 8'hx4) failed = 1;
  $display("%h", array[ 5:4 ]); if (array[ 5:4 ] !== 8'hxx) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
