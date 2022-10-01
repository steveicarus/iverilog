// Check that it is possible to use a forward declared class type as the type of
// a property in another class.

module test;

  typedef class B;

  class C;
    B b;
  endclass

  class B;
  endclass

  C c;

  initial begin
    c = new;
    $display("PASSED");
  end

endmodule
