// Check that `begin_keywords accepts the 1364-1995 keyword set.

`begin_keywords "1364-1995"
module test;
  initial begin
    $display("PASSED");
  end
endmodule
`end_keywords
