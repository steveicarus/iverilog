// Check behaviour with out-of-range and undefined array indices
// on LHS of procedural continuous (net) assignment.

`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  `define SUPPORT_REAL_NETS_IN_IVTEST
`endif

module top;

wire [1:0] array1[2:1];
wire [1:0] array2[1:0];

reg [1:0] var1;

assign array1[1] = 2'd0;
assign array1[2] = 2'd0;

assign array2[0] = 2'd0;
assign array2[1] = 2'd0;

`ifdef SUPPORT_REAL_NETS_IN_IVTEST
wire real array3[2:1];
wire real array4[1:0];

real var2;

assign array3[1] = 0.0;
assign array3[2] = 0.0;

assign array4[0] = 0.0;
assign array4[1] = 0.0;
`endif

reg failed;

initial begin
  failed = 0;

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  force array1[0] = 2'd1;
  #1 $display("array = %h %h", array1[2], array1[1]);
  if ((array1[1] !== 2'd0) || (array1[2] !== 2'd0)) failed = 1;
  release array1[0];
`endif

  force array1[1] = 2'd1;
  #1 $display("array = %h %h", array1[2], array1[1]);
  if ((array1[1] !== 2'd1) || (array1[2] !== 2'd0)) failed = 1;
  release array1[1];

  force array1[2] = var1;
  var1 = 2'd1;
  #1 $display("array = %h %h", array1[2], array1[1]);
  if ((array1[1] !== 2'd0) || (array1[2] !== 2'd1)) failed = 1;
  var1 = 2'd2;
  #1 $display("array = %h %h", array1[2], array1[1]);
  if ((array1[1] !== 2'd0) || (array1[2] !== 2'd2)) failed = 1;
  release array1[2];

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  force array1[3] = var1;
  #1 $display("array = %h %h", array1[2], array1[1]);
  if ((array1[1] !== 2'd0) || (array1[2] !== 2'd0)) failed = 1;
  release array1[3];

  force array2['bx] = 2'd1;
  #1 $display("array = %h %h", array2[1], array2[0]);
  if ((array2[0] !== 2'd0) || (array2[1] !== 2'd0)) failed = 1;
  release array2['bx];
`endif

`ifdef SUPPORT_REAL_NETS_IN_IVTEST
  force array3[0] = 1.0;
  #1 $display("array = %0g %0g", array3[2], array3[1]);
  if ((array3[1] != 0.0) || (array3[2] != 0.0)) failed = 1;
  release array3[0];

  force array3[1] = 1.0;
  #1 $display("array = %0g %0g", array3[2], array3[1]);
  if ((array3[1] != 1.0) || (array3[2] != 0.0)) failed = 1;
  release array3[1];

  force array3[2] = var2;
  var2 = 1.0;
  #1 $display("array = %0g %0g", array3[2], array3[1]);
  if ((array3[1] != 0.0) || (array3[2] != 1.0)) failed = 1;
  var2 = 2.0;
  #1 $display("array = %0g %0g", array3[2], array3[1]);
  if ((array3[1] != 0.0) || (array3[2] != 2.0)) failed = 1;
  release array3[2];

  force array3[3] = var2;
  #1 $display("array = %0g %0g", array3[2], array3[1]);
  if ((array3[1] != 0.0) || (array3[2] != 0.0)) failed = 1;
  release array3[3];

  force array4['bx] = 1.0;
  #1 $display("array = %0g %0g", array4[1], array4[0]);
  if ((array4[0] != 0.0) || (array4[1] != 0.0)) failed = 1;
  release array4['bx];
`endif

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
