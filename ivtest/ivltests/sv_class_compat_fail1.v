// Check that an error is reported when trying to assign an object to variable
// of an unrelated class type

module test;

  class B;
    int x;
  endclass

  class C;
    int y;
  endclass

  B b;
  C c;

  initial begin
    c = new;
    b = c; // This should fail, both classes are unrelated
  end

endmodule
