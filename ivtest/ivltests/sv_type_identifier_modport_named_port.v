// Check that a modport simple port selector can shadow a visible typedef name.

interface I;
  typedef int T;
  logic value;

  modport m(input .T(value));
endinterface

module test;

  I i();

  initial begin
    $display("PASSED");
  end

endmodule
