// Check that class objects can not be used in l-value concatenations.

module test;

  class C;
  endclass

  int x;
  C c;

  initial begin
    {x, c} = x;
  end

endmodule
