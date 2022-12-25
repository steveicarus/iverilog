// Check that an error is reported when trying to use a typed constructor call
// with a type that is not a class.

module test;

  class C;
  endclass

  typedef int T;

  initial begin
    C c;
    c = T::new; // This should fail, T is not a class
    $display("FAILED");
  end

endmodule
