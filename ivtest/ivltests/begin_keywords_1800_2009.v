// Check that `begin_keywords accepts the 1800-2009 keyword set.

`begin_keywords "1800-2009"
module test;
  initial begin
    $display("PASSED");
  end
endmodule
`end_keywords
