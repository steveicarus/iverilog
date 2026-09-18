// Check that a config name can have the same spelling as a visible typedef.

typedef int CFG_NAME;

config CFG_NAME;
  design test;
endconfig

module test;
  initial begin
    $display("PASSED");
  end
endmodule
