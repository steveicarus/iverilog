// Check that enum types declared in a higher level scope are compatible between
// different instances of a module.

typedef enum integer {
  A
} T;

module M;
  T e;
endmodule

module test;
  M m1();
  M m2();

  initial begin
    m1.e = A;
    m2.e = m1.e;
    if (m2.e === A) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
