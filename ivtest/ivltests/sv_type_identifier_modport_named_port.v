// Check that a named modport port can have the same spelling as a visible typedef.

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
