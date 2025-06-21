module test;
  // This should conflict with the generate loops below.
  reg named;
  // scope name matches a register
  for (genvar gv = 0; gv < 1; gv = gv + 1) begin:named; end;

  // You cannot have  generate blocks with the same name.
  for (genvar gv = 0; gv < 1; gv = gv + 1) begin:match; end;
  for (genvar gv = 0; gv < 1; gv = gv + 1) begin:match; end;
endmodule
