// Check that nature names can match visible type identifiers.

typedef int NATURE_NAME;

nature NATURE_NAME;
  units = "V";
  access = NATURE_ACCESS;
endnature

module test;
  initial begin
    $display("PASSED");
  end
endmodule
