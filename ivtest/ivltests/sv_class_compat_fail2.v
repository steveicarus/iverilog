// Check that an error is reported when trying when trying to assign an object
// of a base class to a variable of a inherited class.

module test;

  class B;
    int x;
  endclass

  class C extends B;
    int y;
  endclass

  B b;
  C c;

  initial begin
    b = new;
    c = b; // This should fail, B is a base class of C
  end

endmodule
