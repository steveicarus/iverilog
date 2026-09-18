// Check that a const static property cannot be assigned through an object.

module test;

  class C;
    const static int value = 42;
  endclass

  C object;

  initial begin
    object.value = 1;
  end

endmodule
