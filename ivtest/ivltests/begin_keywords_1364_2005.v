// Check that `begin_keywords accepts the 1364-2005 keyword set.

`begin_keywords "1364-2005"
module test;
  initial begin
    $display("PASSED");
  end
endmodule
`end_keywords
