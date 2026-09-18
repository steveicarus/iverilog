// Check that dimensions cannot follow a type identifier in a cast target.

module test;

  typedef logic [7:0] T;
  logic [7:0] value;

  initial begin
    value = T[3:0]'(1);
  end

endmodule
