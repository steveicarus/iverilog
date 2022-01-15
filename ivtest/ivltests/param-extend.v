module top();

localparam signed [31:0] SizedValue   = -1;
localparam               UnsizedValue = -1;

reg [35:0] Result;
reg        Failed;

initial begin
  Failed = 0;
  // check for sign extension
  Result = SizedValue;
  $display("%h", Result);
  if (Result !== 36'hfffffffff) Failed = 1;
  Result = UnsizedValue;
  $display("%h", Result);
  if (Result !== 36'hfffffffff) Failed = 1;

  // check for zero extension
  Result = 'd0 + SizedValue;
  $display("%h", Result);
  if (Result !== 36'h0ffffffff) Failed = 1;
  Result = 'd0 + UnsizedValue;
  $display("%h", Result);
`ifdef OLD_UNSIZED
  if (Result !== 36'hfffffffff) Failed = 1;
`else
  if (Result !== 36'h0ffffffff) Failed = 1;
`endif

  if (Failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
