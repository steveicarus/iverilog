module pr3561350();

reg [31:0] source;
reg [31:0] result;

initial begin
  source = 10;
  // the following expression results in a compiler internal error
  result = (source * 2) + 2 + 3 + 4;
  // check we get the expected result when the bug has been fixed
  if (result === 29)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
