// Check that bogus member access on procedural l-values reports an error.

module test;
  logic r;

  initial begin
    r.bad = 1'b1;
  end
endmodule
