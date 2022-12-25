// Check that an error is reported when using a typed constructor call to assign
// to a class that is an inherited class.

module test;

  class B;
  endclass

  class C extends B;
  endclass

  initial begin
    C c;
    c = B::new; // This should fail, B is a base class of C, but C is not a
                // base class of B.
    $display("FAILED");
  end

endmodule
