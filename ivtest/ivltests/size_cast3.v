module test();

localparam size1 = 4;
localparam size2 = 6;
localparam size3 = 8;

localparam        [5:0] value1 = 6'h3f;
localparam signed [5:0] value2 = 6'h3f;

reg [31:0] result;

reg failed = 0;

initial begin
  result = size1'(value1) + 'd0;
  $display("%h", result);
  if (result !== 32'h0000000f) failed = 1;

  result = size1'(value1) + 'sd0;
  $display("%h", result);
  if (result !== 32'h0000000f) failed = 1;

  result = size1'(value2) + 'd0;
  $display("%h", result);
  if (result !== 32'h0000000f) failed = 1;

  result = size1'(value2) + 'sd0;
  $display("%h", result);
  if (result !== 32'hffffffff) failed = 1;

  result = size2'(value1) + 'd0;
  $display("%h", result);
  if (result !== 32'h0000003f) failed = 1;

  result = size2'(value1) + 'sd0;
  $display("%h", result);
  if (result !== 32'h0000003f) failed = 1;

  result = size2'(value2) + 'd0;
  $display("%h", result);
  if (result !== 32'h0000003f) failed = 1;

  result = size2'(value2) + 'sd0;
  $display("%h", result);
  if (result !== 32'hffffffff) failed = 1;

  result = size3'(value1) + 'd0;
  $display("%h", result);
  if (result !== 32'h0000003f) failed = 1;

  result = size3'(value1) + 'sd0;
  $display("%h", result);
  if (result !== 32'h0000003f) failed = 1;

  result = size3'(value2) + 'd0;
  $display("%h", result);
  if (result !== 32'h000000ff) failed = 1;

  result = size3'(value2) + 'sd0;
  $display("%h", result);
  if (result !== 32'hffffffff) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule // main
