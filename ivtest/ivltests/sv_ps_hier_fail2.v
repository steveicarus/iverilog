// Check that package scoped identifiers lookup does not cross the package
// boundary.

package P;
endpackage

module test;
  int x;
  initial begin
    int y;
    y = P::test.x; // This should fail. test.x is visible from within the
                   // package, but it can not be accessed through a package
                   // scoped identifier.
    $display("FAILED");
  end
endmodule
