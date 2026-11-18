// Check that $cast() between classes that are not related by
// inheritance is a compile time error.

module test;

  class A;
    int a;
  endclass

  class B;
    int b;
  endclass

  A pa;
  B pb;
  int rc;

  initial begin
    pa = new();
    rc = $cast(pb, pa);
    $display("FAILED");
  end

endmodule
