// Check that `begin_keywords accepts the VAMS-2.3 keyword set.

`begin_keywords "VAMS-2.3"
module test;
  initial begin
    $display("PASSED");
  end
endmodule
`end_keywords
