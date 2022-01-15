// Check behaviour with out-of-range and undefined array indices
// on LHS of continuous assignment.

`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  `define SUPPORT_REAL_NETS_IN_IVTEST
`endif

module top;

wire [1:0] array1[2:1];
wire [1:0] array2[1:0];

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
assign array1[0]   = 2'd0;
`endif
assign array1[1]   = 2'd1;
assign array1[2]   = 2'd2;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
assign array1[3]   = 2'd3;
`endif

assign array2[0]   = 2'd0;
assign array2[1]   = 2'd1;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
assign array2['bx] = 2'd2;
`endif

`ifdef SUPPORT_REAL_NETS_IN_IVTEST
wire real array3[2:1];
wire real array4[1:0];

assign array3[0]   = 0.0;
assign array3[1]   = 1.0;
assign array3[2]   = 2.0;
assign array3[3]   = 3.0;

assign array4[0]   = 0.0;
assign array4[1]   = 1.0;
assign array4['bx] = 2.0;
`endif

reg failed;

initial begin
  #1 failed = 0;

  $display("array = %h %h", array1[2], array1[1]);
  if ((array1[1] !== 2'd1) || (array1[2] !== 2'd2)) failed = 1;

  $display("array = %h %h", array2[1], array2[0]);
  if ((array2[0] !== 2'd0) || (array2[1] !== 2'd1)) failed = 1;

`ifdef SUPPORT_REAL_NETS_IN_IVTEST
  $display("array = %0g %0g", array3[2], array3[1]);
  if ((array3[1] != 1.0) || (array3[2] != 2.0)) failed = 1;

  $display("array = %0g %0g", array4[1], array4[0]);
  if ((array4[0] != 0.0) || (array4[1] != 1.0)) failed = 1;
`endif

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
