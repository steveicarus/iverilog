// Check that it is possible to have multiple instances of a module that
// defines a class derived from a forward declared base class and that the
// actual base class types can have different implementations based on module
// parameters.

module M #(
  parameter X = 0
);

  typedef class B;

  class C extends B;
  endclass

  class B;
    function int f;
      return X;
    endfunction
  endclass

  C c = new;

endmodule

module test;

  M #(10) m1();
  M #(20) m2();

  initial begin
    if (m1.c.f() == 10 && m2.c.f() == 20) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
