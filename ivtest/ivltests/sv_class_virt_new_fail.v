// Check that an error is reported when trying to create an object of a virtual
// class.

module test;

  virtual class C;
  endclass

  C c;

  initial begin
    c = new; // This should fail
  end

endmodule
