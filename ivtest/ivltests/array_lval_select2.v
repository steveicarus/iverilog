// Check behaviour with out-of-range and undefined array indices
// on LHS of non-blocking procedural assignment.

`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;

reg  array1[2:1];
reg  array2[1:0];

`ifndef VLOG95
real array3[2:1];
real array4[1:0];
`endif

integer index;

reg failed;

initial begin
  failed = 0;

  array1[1] <= 1'b0;
  array1[2] <= 1'b0;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  array1[0] <= 1'b1; // Constant out of bounds select may be an error
`endif
  #1 $display("array = %b %b", array1[2], array1[1]);
  if ((array1[1] !== 1'b0) || (array1[2] !== 1'b0)) failed = 1;

  array1[1] <= 1'b0;
  array1[2] <= 1'b0;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  array1[3] <= 1'b1; // Constant out of bounds select may be an error
`endif
  #1 $display("array = %b %b", array1[2], array1[1]);
  if ((array1[1] !== 1'b0) || (array1[2] !== 1'b0)) failed = 1;

  array2[0] <= 1'b0;
  array2[1] <= 1'b0;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  array2['bx] <= 1'b1; // Constant undefined out of bounds select may be an error
`endif
  #1 $display("array = %b %b", array2[1], array2[0]);
  if ((array2[0] !== 1'b0) || (array2[1] !== 1'b0)) failed = 1;

  index = 0;
  array1[1] <= 1'b0;
  array1[2] <= 1'b0;
  array1[index] <= 1'b1;
  #1 $display("array = %b %b", array1[2], array1[1]);
  if ((array1[1] !== 1'b0) || (array1[2] !== 1'b0)) failed = 1;

  index = 3;
  array1[1] <= 1'b0;
  array1[2] <= 1'b0;
  array1[index] <= 1'b1;
  #1 $display("array = %b %b", array1[2], array1[1]);
  if ((array1[1] !== 1'b0) || (array1[2] !== 1'b0)) failed = 1;

  index = 'bx;
  array2[0] <= 1'b0;
  array2[1] <= 1'b0;
  array2[index] <= 1'b1;
  #1 $display("array = %b %b", array2[1], array2[0]);
  if ((array2[0] !== 1'b0) || (array2[1] !== 1'b0)) failed = 1;

`ifndef VLOG95
  array3[1] <= 0.0;
  array3[2] <= 0.0;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  array3[0] <= 1.0; // Constant out of bounds select may be an error
`endif
  #1 $display("array = %0g %0g", array3[2], array3[1]);
  if ((array3[1] != 0.0) || (array3[2] != 0.0)) failed = 1;

  array3[1] <= 0.0;
  array3[2] <= 0.0;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  array3[3] <= 1.0; // Constant out of bounds select may be an error
`endif
  #1 $display("array = %0g %0g", array3[2], array3[1]);
  if ((array3[1] != 0.0) || (array3[2] != 0.0)) failed = 1;

  array4[0] <= 0.0;
  array4[1] <= 0.0;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  array4['bx] <= 1.0; // Constant undefined out of bounds select may be an error
`endif
  #1 $display("array = %0g %0g", array4[1], array4[0]);
  if ((array4[0] != 0.0) || (array4[1] != 0.0)) failed = 1;

  index = 0;
  array3[1] <= 0.0;
  array3[2] <= 0.0;
  array3[index] <= 1.0;
  #1 $display("array = %0g %0g", array3[2], array3[1]);
  if ((array3[1] != 0.0) || (array3[2] != 0.0)) failed = 1;

  index = 3;
  array3[1] <= 0.0;
  array3[2] <= 0.0;
  array3[index] <= 1.0;
  #1 $display("array = %0g %0g", array3[2], array3[1]);
  if ((array3[1] != 0.0) || (array3[2] != 0.0)) failed = 1;

  index = 'bx;
  array4[0] <= 0.0;
  array4[1] <= 0.0;
  array4[index] <= 1.0;
  #1 $display("array = %0g %0g", array4[1], array4[0]);
  if ((array4[0] != 0.0) || (array4[1] != 0.0)) failed = 1;
`endif

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
