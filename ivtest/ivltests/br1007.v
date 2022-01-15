`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module test();

reg [4:1] value;

reg failed;

initial begin
  failed = 0;
  value = 4'b0000;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  value[5] = 1'b1;
`endif
  $display("%b", value);
  if (value !== 4'b0000) failed = 1;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  value[5:5] = 1'b1;
`endif
  $display("%b", value);
  if (value !== 4'b0000) failed = 1;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  value[5:4] = 2'b11;
`else
  value[4] = 1'b1;
`endif
  $display("%b", value);
  if (value !== 4'b1000) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
