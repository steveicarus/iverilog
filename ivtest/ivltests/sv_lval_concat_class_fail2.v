// Check that class objects can not be used in single operand l-value concatenations.

module test;

  class C;
  endclass

  int x;
  C c;

  initial begin
    {c} = x;
  end

endmodule
