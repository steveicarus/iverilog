// Check that `begin_keywords accepts the 1800-2012 keyword set.

`begin_keywords "1800-2012"
module test;
  initial begin
    $display("PASSED");
  end
endmodule
`end_keywords
