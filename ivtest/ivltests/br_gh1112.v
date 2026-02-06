// Test for GitHub issue #1112
// $bits() with non-existent identifier should produce an error
module top;
  localparam width = $bits(value);  // 'value' doesn't exist - should error
  initial $display(width);
endmodule
