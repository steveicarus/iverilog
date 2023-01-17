// Check that enums declared within a module are not compatible between
// different instances of a module

module M;

  enum integer {
    A
  } e;

endmodule

module test;
  M m1();
  M m2();

  initial begin
    // These are different types and not compatible
    m1.e = m2.e;
    $display("FAILED");
  end
endmodule
