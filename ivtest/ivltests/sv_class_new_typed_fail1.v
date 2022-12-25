// Check that an error is reported when using a typed constructor call to assign
// to a class that is not a base class.

module test;

  class B;
  endclass

  class C;
  endclass

  initial begin
    B b;
    b = C::new; // This should fail, B is not a base class of C
    $display("FAILED");
  end

endmodule
