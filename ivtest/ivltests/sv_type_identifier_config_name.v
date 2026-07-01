// Check that a config name can shadow a visible type identifier.

typedef int CFG_NAME;

config CFG_NAME;
  design test;
endconfig

module test;
  initial begin
    $display("PASSED");
  end
endmodule
