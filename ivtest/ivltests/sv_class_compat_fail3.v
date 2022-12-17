// Check that an error is reported when trying to assign an object to a variable
// that shares a common base class but is not directly related.

module test;

  class B;
    int x;
  endclass

  class C extends B;
    int y;
  endclass

  class D extends B;
    int z;
  endclass

  C c;
  D d;

  initial begin
    c = new;
    d = c; // This should fail, both C and D inherit from B, but there is no
           // direct relationship.
  end

endmodule
