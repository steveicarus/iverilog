// Check that an error is reported when using a typed constructor call to assign
// to a class that has a common base class, but is not directly related.

module test;

  class B;
  endclass

  class C extends B;
  endclass

  class D extends B;
  endclass

  initial begin
    D d;
    d = C::new; // This should fail, C and D share a common base class, but are
                // not compatible
    $display("FAILED");
  end

endmodule
